/*
 * Copyright (c) 2002-2004 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2004 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/releases/SuperG_Linux_AP/src/802_11/madwifi/hal/ar5312/ar5312_reset.c#5 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5312

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5312/ar5312phy.h"

/* Add static register initialization vectors */
#define AH_5212_COMMON
#include "ar5212/ar5212.ini"

/* Additional Time delay to wait after activiting the Base band */
#define BASE_ACTIVATE_DELAY	100	/* 100 usec */
#define PLL_SETTLE_DELAY	300	/* 300 usec */

extern int16_t ar5212GetNf(struct ath_hal *, HAL_CHANNEL_INTERNAL *);
extern void ar5212SetRateDurationTable(struct ath_hal *, HAL_CHANNEL *);
extern HAL_BOOL ar5212SetTransmitPower(struct ath_hal *ah,
                      HAL_CHANNEL_INTERNAL *chan, u_int16_t *rfXpdGain);
extern void ar5212SetDeltaSlope(struct ath_hal *, HAL_CHANNEL *);
extern HAL_BOOL ar5212SetBoardValues(struct ath_hal *, HAL_CHANNEL_INTERNAL *);
extern void ar5212SetOperatingMode(struct ath_hal *, int );

#ifdef AH_SUPPORT_XR
extern HAL_BOOL ar5212SetXrMode(struct ath_hal *ah, HAL_OPMODE opmode)
#endif

static HAL_BOOL ar5312SetResetReg(struct ath_hal *, u_int32_t resetMask);

/*
 * WAR for bug 6773.  OS_DELAY() does a PIO READ on the PCI bus which allows
 * other cards' DMA reads to complete in the middle of our reset.
 */
#define WAR_6773(x) do {		\
	if ((++(x) % 64) == 0)		\
		OS_DELAY(1);		\
} while (0)

#define REG_WRITE_ARRAY(regArray, column, regWr) do {                  	\
	int r;								\
	for (r = 0; r < N(regArray); r++) {				\
		OS_REG_WRITE(ah, (regArray)[r][0], (regArray)[r][(column)]);\
		WAR_6773(regWr);					\
	}								\
} while (0)

/*
 * Places the device in and out of reset and then places sane
 * values in the registers based on EEPROM config, initialization
 * vectors (as determined by the mode), and station configuration
 *
 * bChannelChange is used to preserve DMA/PCU registers across
 * a HW Reset during channel change.
 */
HAL_BOOL
ar5312Reset(struct ath_hal *ah, HAL_OPMODE opmode,
	HAL_CHANNEL *chan, HAL_BOOL bChannelChange, HAL_STATUS *status)
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
#define	FAIL(_code)	do { ecode = _code; goto bad; } while (0)
	u_int32_t softLedCfg, softLedState;
	struct ath_hal_5212 *ahp = AH5212(ah);
	HAL_CHANNEL_INTERNAL *ichan;
	u_int32_t saveFrameSeqCount, saveDefAntenna, saveLedState;
	u_int32_t macStaId1, synthDelay;
	u_int16_t rfXpdGain[2];
	int16_t cckOfdmPwrDelta = 0;
	u_int modesIndex, freqIndex;
	HAL_STATUS ecode;
	int i, regWrites = 0;

	OS_MARK(ah, AH_MARK_RESET, bChannelChange);
#define	IS(_c,_f)	(((_c)->channelFlags & _f) || 0)
	if ((IS(chan, CHANNEL_2GHZ) ^ IS(chan, CHANNEL_5GHZ)) == 0) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; not marked as "
			"2GHz or 5GHz\n", __func__,
			chan->channel, chan->channelFlags);
		FAIL(HAL_EINVAL);
	}
	if ((IS(chan, CHANNEL_OFDM) ^ IS(chan, CHANNEL_CCK)) == 0) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; not marked as "
			"OFDM or CCK\n", __func__,
			chan->channel, chan->channelFlags);
		FAIL(HAL_EINVAL);
	}
#undef IS
	/*
	 * Map public channel to private.
	 */
	ichan = ath_hal_checkchannel(ah, chan);
	if (ichan == AH_NULL) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; no mapping\n",
			__func__, chan->channel, chan->channelFlags);
		FAIL(HAL_EINVAL);
	}
	switch (opmode) {
	case HAL_M_STA:
	case HAL_M_IBSS:
	case HAL_M_HOSTAP:
	case HAL_M_MONITOR:
		break;
	default:
		HALDEBUG(ah, "%s: invalid operating mode %u\n",
			__func__, opmode);
		FAIL(HAL_EINVAL);
		break;
	}
	HALASSERT(ahp->ah_eeversion >= AR_EEPROM_VER3);

	/* Preserve certain DMA hardware registers on a channel change */
	if (bChannelChange) {
		/*
		 * AR5212 WAR
		 *
		 * On Venice, the TSF is almost preserved across a reset;
		 * it requires the WAR of doubling writes to the RESET_TSF
		 * bit in the AR_BEACON register; it also has the quirk
		 * of the TSF going back in time on the station (station
		 * latches onto the last beacon's tsf during a reset 50%
		 * of the times); the latter is not a problem for adhoc
		 * stations since as long as the TSF is behind, it will
		 * get resynchronized on receiving the next beacon; the
		 * TSF going backwards in time could be a problem for the
		 * sleep operation (supported on infrastructure stations
		 * only) - the best and most general fix for this situation
		 * is to resynchronize the various sleep/beacon timers on
		 * the receipt of the next beacon i.e. when the TSF itself
		 * gets resynchronized to the AP's TSF - power save is
		 * needed to be temporarily disabled until that time
		 *
		 * Need to save the sequence number to restore it after
		 * the reset!
		 */
		saveFrameSeqCount = OS_REG_READ(ah, AR_D_SEQNUM);

		ar5212GetNf(ah, ichan);
	} else
		saveFrameSeqCount = 0;		/* NB: silence compiler */

	/*
	 * Preserve the antenna on a channel change
	 */
	saveDefAntenna = OS_REG_READ(ah, AR_DEF_ANTENNA);
	if (saveDefAntenna == 0)		/* XXX magic constants */
		saveDefAntenna = 1;

	/* Save hardware flag before chip reset clears the register */
	macStaId1 = OS_REG_READ(ah, AR_STA_ID1) & AR_STA_ID1_BASE_RATE_11B;
#if !AH_SUPPORT_2316 /* not yet */
	/* Save led state from pci config register */
	saveLedState = OS_REG_READ(ah, AR531X_PCICFG) &
		(AR_PCICFG_LEDCTL | AR_PCICFG_LEDMODE | AR_PCICFG_LEDBLINK |
		 AR_PCICFG_LEDSLOW);
#endif


	ar5312RestoreClock(ah, opmode);		/* move to refclk operation */

	/*
	 * Adjust gain parameters before reset if
	 * there's an outstanding gain updated.
	 */
	(void) ar5212GetRfgain(ah);

	if (!ar5312ChipReset(ah, chan)) {
		HALDEBUG(ah, "%s: chip reset failed\n", __func__);
		FAIL(HAL_EIO);
	}

	/* Setup the indices for the next set of register array writes */
	switch (chan->channelFlags & CHANNEL_ALL) {
	case CHANNEL_A:
	case CHANNEL_XR_A:
		modesIndex = 1;
		freqIndex  = 1;
		break;
	case CHANNEL_T:
	case CHANNEL_XR_T:
		modesIndex = 2;
		freqIndex  = 1;
		break;
	case CHANNEL_B:
		modesIndex = 3;
		freqIndex  = 2;
		break;
	case CHANNEL_PUREG:
	case CHANNEL_XR_G:
		modesIndex = 4;
		freqIndex  = 2;
		break;
	case CHANNEL_108G:
		modesIndex = 5;
		freqIndex  = 2;
		break;
	default:
		HALDEBUG(ah, "%s: invalid channel flags 0x%x\n",
			__func__, chan->channelFlags);
		FAIL(HAL_EINVAL);
	}

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	/* Set correct Baseband to analog shift setting to access analog chips. */
	OS_REG_WRITE(ah, AR_PHY(0), 0x00000007);

	REG_WRITE_ARRAY(ar5212Modes, modesIndex, regWrites);
	/* Write Common Array Parameters */
	for (i = 0; i < N(ar5212Common); i++) {
		u_int32_t reg = ar5212Common[i][0];
		/* XXX timer/beacon setup registers? */
		/* On channel change, don't reset the PCU registers */
		if (!(bChannelChange && (0x8000 <= reg && reg < 0x9000))) {
			OS_REG_WRITE(ah, reg, ar5212Common[i][1]);
			WAR_6773(regWrites);
		}
	}
	ahp->ah_rfHal.writeRegs(ah, modesIndex, freqIndex, regWrites);

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	/* Setup multi-rate retry table */
	ar5212SetRateDurationTable(ah, chan);

	/* Overwrite INI values for revised chipsets */
	if (AH_PRIVATE(ah)->ah_phyRev >= AR_PHY_CHIP_ID_REV_2) {
		/* ADC_CTL */
		OS_REG_WRITE(ah, AR_PHY_ADC_CTL,
			     SM(2, AR_PHY_ADC_CTL_OFF_INBUFGAIN) |
			     SM(2, AR_PHY_ADC_CTL_ON_INBUFGAIN) |
			     AR_PHY_ADC_CTL_OFF_PWDDAC |
			     AR_PHY_ADC_CTL_OFF_PWDBANDGAP |
			     AR_PHY_ADC_CTL_OFF_PWDADC);    
		
		/* TX_PWR_ADJ */
		if (chan->channel == 2484) {
			cckOfdmPwrDelta = SCALE_OC_DELTA(ahp->ah_cckOfdmPwrDelta - ahp->ah_scaledCh14FilterCckDelta);
		} else {
			cckOfdmPwrDelta = SCALE_OC_DELTA(ahp->ah_cckOfdmPwrDelta);
		}
		
		if (IS_CHAN_G(chan)) {
			OS_REG_WRITE(ah, AR_PHY_TXPWRADJ,
				     SM((ahp->ah_cckOfdmPwrDelta*-1), AR_PHY_TXPWRADJ_CCK_GAIN_DELTA) |
				     SM((cckOfdmPwrDelta*-1), AR_PHY_TXPWRADJ_CCK_PCDAC_INDEX));
		} else {
			OS_REG_WRITE(ah, AR_PHY_TXPWRADJ, 0);
		}
		
		/* Add barker RSSI thresh enable as disabled */
		OS_REG_CLR_BIT(ah, AR_PHY_DAG_CTRLCCK,
			       AR_PHY_DAG_CTRLCCK_EN_RSSI_THR);
		OS_REG_RMW_FIELD(ah, AR_PHY_DAG_CTRLCCK,
				 AR_PHY_DAG_CTRLCCK_RSSI_THR, 2);
		
		/* Set the mute mask to the correct default */
		OS_REG_WRITE(ah, AR_SEQ_MASK, 0x0000000F);
	}
	
	if (AH_PRIVATE(ah)->ah_phyRev >= AR_PHY_CHIP_ID_REV_3) {
		/* Clear reg to alllow RX_CLEAR line debug */
		OS_REG_WRITE(ah, AR_PHY_BLUETOOTH,  0);
	}
	if (AH_PRIVATE(ah)->ah_phyRev >= AR_PHY_CHIP_ID_REV_4) {
#ifdef notyet
		/* Enable burst prefetch for the data queues */
		OS_REG_RMW_FIELD(ah, AR_D_FPCTL, ... );
		/* Enable double-buffering */
		OS_REG_CLR_BIT(ah, AR_TXCFG, AR_TXCFG_DBL_BUF_DIS);
#endif
	}

        /* Set ADC/DAC select values */
        OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL, 0x0e);

        /* ADC_CTRL */
        OS_REG_WRITE(ah, AR_PHY_SIGMA_DELTA,
                     SM(1, AR_PHY_SIGMA_DELTA_ADC_SEL) |
                     SM(4, AR_PHY_SIGMA_DELTA_FILT2) |
                     SM(0x16, AR_PHY_SIGMA_DELTA_FILT1) |
                     SM(5, AR_PHY_SIGMA_DELTA_ADC_CLIP));

	/* Setup the transmit power values. */
	if (!ar5212SetTransmitPower(ah, ichan, rfXpdGain)) {
		HALDEBUG(ah, "%s: error init'ing transmit power\n", __func__);
		FAIL(HAL_EIO);
	}

	/* Write the analog registers */
	if (!ahp->ah_rfHal.setRfRegs(ah, ichan, modesIndex, rfXpdGain)) {
		HALDEBUG(ah, "%s: ar5212SetRfRegs failed\n", __func__);
		FAIL(HAL_EIO);
	}

	/* Write delta slope for OFDM enabled modes (A, G, Turbo) */
	if (IS_CHAN_OFDM(chan))
		ar5212SetDeltaSlope(ah, chan);

	/* Setup board specific options for EEPROM version 3 */
	if (!ar5212SetBoardValues(ah, ichan)) {
		HALDEBUG(ah, "%s: error setting board options\n", __func__);
		FAIL(HAL_EIO);
	}

	/* Restore certain DMA hardware registers on a channel change */
	if (bChannelChange)
		OS_REG_WRITE(ah, AR_D_SEQNUM, saveFrameSeqCount);

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	OS_REG_WRITE(ah, AR_STA_ID0, LE_READ_4(ahp->ah_macaddr));
	OS_REG_WRITE(ah, AR_STA_ID1, LE_READ_2(ahp->ah_macaddr + 4)
		| macStaId1
		| AR_STA_ID1_RTS_USE_DEF
		| (ahp->ah_micEnabled ? AR_STA_ID1_CRPT_MIC_ENABLE : 0));
	ar5212SetOperatingMode(ah, opmode);

	/* Set Venice BSSID mask for one bssid */
	OS_REG_WRITE(ah, AR_BSSMSKL, 0xffffffff);
	OS_REG_WRITE(ah, AR_BSSMSKU, 0xffffffff);
#if !AH_SUPPORT_2316 /* not yet */
	/* Restore previous led state */
	OS_REG_WRITE(ah, AR531X_PCICFG, OS_REG_READ(ah, AR_PCICFG) | saveLedState);
#endif

	/* Restore previous antenna */
	OS_REG_WRITE(ah, AR_DEF_ANTENNA, saveDefAntenna);

	/* then our BSSID */
	OS_REG_WRITE(ah, AR_BSS_ID0, LE_READ_4(ahp->ah_bssid));
	OS_REG_WRITE(ah, AR_BSS_ID1, LE_READ_2(ahp->ah_bssid + 4));

	OS_REG_WRITE(ah, AR_ISR, ~0);		/* cleared on write */

	OS_REG_WRITE(ah, AR_RSSI_THR, INIT_RSSI_THR);

	if (!ahp->ah_rfHal.setChannel(ah, ichan))
		FAIL(HAL_EIO);

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	/*
	 * Setup fast diversity.
	 * Fast diversity can be enabled or disabled via regadd.txt.
	 * Default is enabled.
	 * For reference,
	 *    Disable: reg        val
	 *             0x00009860 0x00009d18 (if 11a / 11g, else no change)
	 *             0x00009970 0x192bb514
	 *             0x0000a208 0xd03e4648
	 *
	 *    Enable:  0x00009860 0x00009d10 (if 11a / 11g, else no change)
	 *             0x00009970 0x192fb514
	 *             0x0000a208 0xd03e6788
	 */

	/* XXX Setup pre PHY ENABLE EAR additions */
	/*
	 * Wait for the frequency synth to settle (synth goes on
	 * via AR_PHY_ACTIVE_EN).  Read the phy active delay register.
	 * Value is in 100ns increments.
	 */
	synthDelay = OS_REG_READ(ah, AR_PHY_RX_DELAY) & AR_PHY_RX_DELAY_DELAY;
	if (IS_CHAN_CCK(chan)) {
		synthDelay = (4 * synthDelay) / 22;
	} else {
		synthDelay /= 10;
	}

	/* Activate the PHY (includes baseband activate and synthesizer on) */
	OS_REG_WRITE(ah, AR_PHY_ACTIVE, AR_PHY_ACTIVE_EN);

	/* 
	 * There is an issue if the AP starts the calibration before
	 * the base band timeout completes.  This could result in the
	 * rx_clear false triggering.  As a workaround we add delay an
	 * extra BASE_ACTIVATE_DELAY usecs to ensure this condition
	 * does not happen.
	 */
	OS_DELAY(synthDelay + BASE_ACTIVATE_DELAY);

	/* Calibrate the AGC and start a NF calculation */
	OS_REG_WRITE(ah, AR_PHY_AGC_CONTROL,
		  OS_REG_READ(ah, AR_PHY_AGC_CONTROL)
		| AR_PHY_AGC_CONTROL_CAL
		| AR_PHY_AGC_CONTROL_NF);

	if (!IS_CHAN_B(chan) && ahp->ah_bIQCalibration != IQ_CAL_DONE) {
		/* Start IQ calibration w/ 2^(INIT_IQCAL_LOG_COUNT_MAX+1) samples */
		OS_REG_RMW_FIELD(ah, AR_PHY_TIMING_CTRL4, 
			AR_PHY_TIMING_CTRL4_IQCAL_LOG_COUNT_MAX,
			INIT_IQCAL_LOG_COUNT_MAX);
		OS_REG_SET_BIT(ah, AR_PHY_TIMING_CTRL4,
			AR_PHY_TIMING_CTRL4_DO_IQCAL);
		ahp->ah_bIQCalibration = IQ_CAL_RUNNING;
	} else
		ahp->ah_bIQCalibration = IQ_CAL_INACTIVE;

	/* XXX Setup compression registers */

	/* Set 1:1 QCU to DCU mapping for all queues */
	for (i = 0; i < AR_NUM_DCU; i++)
		OS_REG_WRITE(ah, AR_DQCUMASK(i), 1 << i);

	ahp->ah_intrTxqs = 0;
	for (i = 0; i < AH_PRIVATE(ah)->ah_caps.halTotalQueues; i++)
		ar5212ResetTxQueue(ah, i);

	/*
	 * Setup interrupt handling.  Note that ar5212ResetTxQueue
	 * manipulates the secondary IMR's as queues are enabled
	 * and disabled.  This is done with RMW ops to insure the
	 * settings we make here are preserved.
	 */
	ahp->ah_maskReg = AR_IMR_TXOK | AR_IMR_TXERR | AR_IMR_TXURN
			| AR_IMR_RXOK | AR_IMR_RXERR | AR_IMR_RXORN
			| AR_IMR_HIUERR
			;
	if (opmode == HAL_M_HOSTAP)
		ahp->ah_maskReg |= AR_IMR_MIB;
	OS_REG_WRITE(ah, AR_IMR, ahp->ah_maskReg);
	/* Enable bus errors that are OR'd to set the HIUERR bit */
	OS_REG_WRITE(ah, AR_IMR_S2,
		OS_REG_READ(ah, AR_IMR_S2)
		| AR_IMR_S2_MCABT | AR_IMR_S2_SSERR | AR_IMR_S2_DPERR);

	if (ar5212GetRfKill(ah))
		ar5212EnableRfKill(ah);

	if (!ath_hal_wait(ah, AR_PHY_AGC_CONTROL, AR_PHY_AGC_CONTROL_CAL, 0)) {
		HALDEBUG(ah, "%s: offset calibration failed to complete in 1ms;"
			" noisy environment?\n", __func__);
	}

	/* Turn on NOACK Support for QoS packets */
	OS_REG_WRITE(ah, AR_NOACK,
		((2 << AR_NOACK_2BIT_VALUE_S) & AR_NOACK_2BIT_VALUE) | 
		((5 << AR_NOACK_BIT_OFFSET_S) & AR_NOACK_BIT_OFFSET) | 
		((0 << AR_NOACK_BYTE_OFFSET_S) & AR_NOACK_BYTE_OFFSET)); 		 

	/*
	 * Set clocks back to 32kHz if they had been using refClk, then
	 * use an external 32kHz crystal when sleeping, if one exists.
	 */
	ar5312SetupClock(ah, opmode);

	/*
	 * Writing to AR_BEACON will start timers. Hence it should
	 * be the last register to be written. Do not reset tsf, do
	 * not enable beacons at this point, but preserve other values
	 * like beaconInterval.
	 */
	OS_REG_WRITE(ah, AR_BEACON,
		(OS_REG_READ(ah, AR_BEACON) &~ (AR_BEACON_EN | AR_BEACON_RESET_TSF)));

	/* XXX Setup post reset EAR additions */

#ifdef AH_SUPPORT_XR
	if (IS_CHAN_XR(chan) && !ar5212SetXrMode(ah, opmode)) {
		HALDEBUG(ah, "%s: unable to setup XR mode\n", __func__);
		FAIL(HAL_EIO);
	}
#endif /* AH_SUPPORT_XR */

	/* XXX Qos support */

	/* Restore user-specified settings */
	if (ahp->ah_slottime != (u_int) -1)
		ar5212SetSlotTime(ah, ahp->ah_slottime);
	if (ahp->ah_acktimeout != (u_int) -1)
		ar5212SetAckTimeout(ah, ahp->ah_acktimeout);
	if (ahp->ah_ctstimeout != (u_int) -1)
		ar5212SetCTSTimeout(ah, ahp->ah_ctstimeout);
	if (AH_PRIVATE(ah)->ah_diagreg != 0)
		OS_REG_WRITE(ah, AR_DIAG_SW, AH_PRIVATE(ah)->ah_diagreg);

	AH_PRIVATE(ah)->ah_opmode = opmode;	/* record operating mode */

	HALDEBUG(ah, "%s: done\n", __func__);

	OS_MARK(ah, AH_MARK_RESET_DONE, 0);

	return AH_TRUE;
bad:
	OS_MARK(ah, AH_MARK_RESET_DONE, ecode);
	if (*status)
		*status = ecode;
	return AH_FALSE;
#undef FAIL
#undef N
}

/*
 * Places the PHY and Radio chips into reset.  A full reset
 * must be called to leave this state.  The PCI/MAC/PCU are
 * not placed into reset as we must receive interrupt to
 * re-enable the hardware.
 */
HAL_BOOL
ar5312PhyDisable(struct ath_hal *ah)
{
    return ar5312SetResetReg(ah, AR_RC_BB);
}

/*
 * Places all of hardware into reset
 */
HAL_BOOL
ar5312Disable(struct ath_hal *ah)
{
	if (!ar5312SetPowerMode(ah, HAL_PM_AWAKE, AH_TRUE, 0))
		return AH_FALSE;
	/*
	 * Reset the HW - PCI must be reset after the rest of the
	 * device has been reset.
	 */
	return ar5312SetResetReg(ah, AR_RC_MAC | AR_RC_BB);
}

/*
 * Places the hardware into reset and then pulls it out of reset
 *
 * TODO: Only write the PLL if we're changing to or from CCK mode
 * 
 * WARNING: The order of the PLL and mode registers must be correct.
 */
HAL_BOOL
ar5312ChipReset(struct ath_hal *ah, HAL_CHANNEL *chan)
{

	OS_MARK(ah, AH_MARK_CHIPRESET, chan ? chan->channel : 0);

	/*
	 * Reset the HW 
	 */
	if (!ar5312SetResetReg(ah, AR_RC_MAC | AR_RC_BB))
		return AH_FALSE;

	/* Bring out of sleep mode (AGAIN) */
	if (!ar5312SetPowerMode(ah, HAL_PM_AWAKE, AH_TRUE, 0))
		 return AH_FALSE;

	/* Clear warm reset register */
	if (!ar5312SetResetReg(ah, 0))
		return AH_FALSE;

	/*
	 * Perform warm reset before the mode/PLL/turbo registers
	 * are changed in order to deactivate the radio.  Mode changes
	 * with an active radio can result in corrupted shifts to the
	 * radio device.
	 */

	/*
	 * Set CCK and Turbo modes correctly.
	 */
	if (chan != AH_NULL) {		/* NB: can be null during attach */
		u_int32_t rfMode, phyPLL, curPhyPLL, turbo;

		if (IS_5112(ah) || IS_2413(ah)) {
			rfMode = AR_PHY_MODE_AR5112;
			phyPLL = ((IS_CHAN_CCK(chan) || IS_CHAN_G(chan))?
				  AR_PHY_PLL_CTL_44_5112 : 
				  AR_PHY_PLL_CTL_40_5112);
		} else {
			rfMode = AR_PHY_MODE_AR5111;
			phyPLL = ((IS_CHAN_CCK(chan) || IS_CHAN_G(chan))?
				  AR_PHY_PLL_CTL_44 : 
				  AR_PHY_PLL_CTL_40);
		}
		if (IS_CHAN_OFDM(chan) && (IS_CHAN_CCK(chan) || 
					   IS_CHAN_G(chan)))
			rfMode |= AR_PHY_MODE_DYNAMIC;
		else if (IS_CHAN_OFDM(chan))
			rfMode |= AR_PHY_MODE_OFDM;
		else
			rfMode |= AR_PHY_MODE_CCK;
		if (IS_CHAN_5GHZ(chan))
			rfMode |= AR_PHY_MODE_RF5GHZ;
		else
			rfMode |= AR_PHY_MODE_RF2GHZ;
		turbo = IS_CHAN_TURBO(chan) ?
			(AR_PHY_FC_TURBO_MODE | AR_PHY_FC_TURBO_SHORT) : 0;
		curPhyPLL = OS_REG_READ(ah, AR_PHY_PLL_CTL);

		/*
		 * PLL, Mode, and Turbo values must be written in the correct
		 * order to ensure:
		 * - The PLL cannot be set to 44 unless the CCK or DYNAMIC
		 *   mode bit is set
		 * - Turbo cannot be set at the same time as CCK or DYNAMIC
		 */
		if (IS_CHAN_CCK(chan) || IS_CHAN_G(chan)) {
			OS_REG_WRITE(ah, AR_PHY_TURBO, turbo);
			OS_REG_WRITE(ah, AR_PHY_MODE, rfMode);
			if (curPhyPLL != phyPLL) {
				OS_REG_WRITE(ah,  AR_PHY_PLL_CTL,  phyPLL);
				/* Wait for the PLL to settle */
				OS_DELAY(PLL_SETTLE_DELAY);
			}
		} else {
			if (curPhyPLL != phyPLL) {
				OS_REG_WRITE(ah,  AR_PHY_PLL_CTL,  phyPLL);
				/* Wait for the PLL to settle */
				OS_DELAY(PLL_SETTLE_DELAY);
			}
			OS_REG_WRITE(ah, AR_PHY_TURBO, turbo);
			OS_REG_WRITE(ah, AR_PHY_MODE, rfMode);
		}
	}
	return AH_TRUE;
}

/*
 * Write the given reset bit mask into the reset register
 */
static HAL_BOOL
ar5312SetResetReg(struct ath_hal *ah, u_int32_t resetMask)
{
	u_int32_t mask = resetMask ? resetMask : ~0;
	HAL_BOOL rt;

        if ((rt = ar5312MacReset(ah, mask)) == AH_FALSE) {
		return rt;
	}
        if ((resetMask & AR_RC_MAC) == 0) {
		if (isBigEndian()) {
			/*
			 * Set CFG, little-endian for register
			 * and descriptor accesses.
			 */
			mask = INIT_CONFIG_STATUS |
                                AR_CFG_SWTD | AR_CFG_SWRD;
			OS_REG_WRITE(ah, AR_CFG, mask);
		} else
			OS_REG_WRITE(ah, AR_CFG, INIT_CONFIG_STATUS);
	}
	return rt;
}

/*
 * ar5312MacReset resets (and then un-resets) the specified
 * wireless components.
 * Note: The RCMask cannot be zero on entering from ar5312SetResetReg.
 */

HAL_BOOL
ar5312MacReset(struct ath_hal *ah, unsigned int RCMask)
{
	int wlanNum;
	u_int32_t resetBB, resetBits, regMask;
	u_int32_t reg;

	if (RCMask == 0)
		return(AH_FALSE);
        if ((wlanNum = GETWMACNUM(ah)) == -1) {
                return (AH_FALSE);
        }
#if AH_SUPPORT_2316
	    if(AH_PRIVATE(ah)->ah_devid ==  AR5212_AR5312_REV8 ) {
  
			switch(wlanNum) {
			case 0:
				resetBB = AR5315_RC_BB0_CRES | AR5315_RC_WBB0_RES; 
				/* Warm and cold reset bits for wbb */
				resetBits = AR5315_RC_WMAC0_RES;
				break;
			case 1:
				resetBB = AR5315_RC_BB1_CRES | AR5315_RC_WBB1_RES; 
				/* Warm and cold reset bits for wbb */
				resetBits = AR5315_RC_WMAC1_RES;
				break;
			default:
				return(AH_FALSE);
			}		
			regMask = ~(resetBB | resetBits);

			/* read before */
			reg = OS_REG_READ(ah, 
							  (AR5315_RSTIMER_BASE - ((u_int32_t) ah->ah_sh) + AR5315_RESET));

			if (RCMask == AR_RC_BB) {
				/* Put baseband in reset */
				reg |= resetBB;    /* Cold and warm reset the baseband bits */
			} else {
				/*
				 * Reset the MAC and baseband.  This is a bit different than
				 * the PCI version, but holding in reset causes problems.
				 */
				reg &= regMask;
				reg |= (resetBits | resetBB) ;
			}
			OS_REG_WRITE(ah, 
						 (AR5315_RSTIMER_BASE - ((u_int32_t) ah->ah_sh)+AR5315_RESET),
						 reg);
			/* read after */
			OS_REG_READ(ah, 
						(AR5315_RSTIMER_BASE - ((u_int32_t) ah->ah_sh) +AR5315_RESET));
			OS_DELAY(100);

			/* Bring MAC and baseband out of reset */
			reg &= regMask;
			/* read before */
			OS_REG_READ(ah, 
						(AR5315_RSTIMER_BASE- ((u_int32_t) ah->ah_sh) +AR5315_RESET));
			OS_REG_WRITE(ah, 
						 (AR5315_RSTIMER_BASE - ((u_int32_t) ah->ah_sh)+AR5315_RESET),
						 reg);
			/* read after */
			OS_REG_READ(ah,
						(AR5315_RSTIMER_BASE- ((u_int32_t) ah->ah_sh) +AR5315_RESET));


		} 
        else 
#endif
		{

			switch(wlanNum) {
			case 0:
				resetBB = AR_RC_BB0_CRES | AR_RC_WBB0_RES; 
				/* Warm and cold reset bits for wbb */
				resetBits = AR_RC_WMAC0_RES;
				break;
			case 1:
				resetBB = AR_RC_BB1_CRES | AR_RC_WBB1_RES; 
				/* Warm and cold reset bits for wbb */
				resetBits = AR_RC_WMAC1_RES;
				break;
			default:
				return(AH_FALSE);
			}		
			regMask = ~(resetBB | resetBits);

			/* read before */
			reg = OS_REG_READ(ah, 
							  (AR_RSTIMER_BASE - ((u_int32_t) ah->ah_sh) + AR531X_RESET));

			if (RCMask == AR_RC_BB) {
				/* Put baseband in reset */
				reg |= resetBB;    /* Cold and warm reset the baseband bits */
			} else {
				/*
				 * Reset the MAC and baseband.  This is a bit different than
				 * the PCI version, but holding in reset causes problems.
				 */
				reg &= regMask;
				reg |= (resetBits | resetBB) ;
			}
			OS_REG_WRITE(ah, 
						 (AR_RSTIMER_BASE - ((u_int32_t) ah->ah_sh)+AR531X_RESET),
						 reg);
			/* read after */
			OS_REG_READ(ah, 
						(AR_RSTIMER_BASE - ((u_int32_t) ah->ah_sh) +AR531X_RESET));
			OS_DELAY(100);

			/* Bring MAC and baseband out of reset */
			reg &= regMask;
			/* read before */
			OS_REG_READ(ah, 
						(AR_RSTIMER_BASE- ((u_int32_t) ah->ah_sh) +AR531X_RESET));
			OS_REG_WRITE(ah, 
						 (AR_RSTIMER_BASE - ((u_int32_t) ah->ah_sh)+AR531X_RESET),
						 reg);
			/* read after */
			OS_REG_READ(ah,
						(AR_RSTIMER_BASE- ((u_int32_t) ah->ah_sh) +AR531X_RESET));
		}
	return(AH_TRUE);
}

#endif /* AH_SUPPORT_AR5312 */
