/******************************************************************************
 * rtl871x_mp_ioctl.c
 *
 * Copyright(c) 2007 - 2010 Realtek Corporation. All rights reserved.
 * Linux device driver for RTL8192SU
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * Modifications for inclusion into the Linux staging tree are
 * Copyright(c) 2010 Larry Finger. All rights reserved.
 *
 * Contact information:
 * WLAN FAE <wlanfae@realtek.com>
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 ******************************************************************************/

#include <linux/rndis.h>
#include "osdep_service.h"
#include "drv_types.h"
#include "mlme_osdep.h"
#include "rtl871x_mp.h"
#include "rtl871x_mp_ioctl.h"

uint oid_null_function(struct oid_par_priv *poid_par_priv)
{
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_wireless_mode_hdl(struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid == SET_OID) {
		if (poid_par_priv->information_buf_len >= sizeof(u8))
			Adapter->registrypriv.wireless_mode =
					*(u8 *)poid_par_priv->information_buf;
		else
			status = RNDIS_STATUS_INVALID_LENGTH;
	} else if (poid_par_priv->type_of_oid == QUERY_OID) {
		if (poid_par_priv->information_buf_len >= sizeof(u8)) {
			*(u8 *)poid_par_priv->information_buf =
					 Adapter->registrypriv.wireless_mode;
			*poid_par_priv->bytes_rw =
					poid_par_priv->information_buf_len;
		} else
			status = RNDIS_STATUS_INVALID_LENGTH;
	} else {
		status = RNDIS_STATUS_NOT_ACCEPTED;
	}
	return status;
}

uint oid_rt_pro_write_bb_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct bb_reg_param *pbbreg;
	u16 offset;
	u32 value;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(struct bb_reg_param))
		return RNDIS_STATUS_INVALID_LENGTH;
	pbbreg = (struct bb_reg_param *)(poid_par_priv->information_buf);
	offset = (u16)(pbbreg->offset) & 0xFFF; /*0ffset :0x800~0xfff*/
	if (offset < BB_REG_BASE_ADDR)
		offset |= BB_REG_BASE_ADDR;
	value = pbbreg->value;
	r8712_bb_reg_write(Adapter, offset, value);
	return status;
}

uint oid_rt_pro_read_bb_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct bb_reg_param *pbbreg;
	u16 offset;
	u32 value;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(struct bb_reg_param))
		return RNDIS_STATUS_INVALID_LENGTH;
	pbbreg = (struct bb_reg_param *)(poid_par_priv->information_buf);
	offset = (u16)(pbbreg->offset) & 0xFFF; /*0ffset :0x800~0xfff*/
	if (offset < BB_REG_BASE_ADDR)
		offset |= BB_REG_BASE_ADDR;
	value = r8712_bb_reg_read(Adapter, offset);
	pbbreg->value = value;
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}

uint oid_rt_pro_write_rf_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct rf_reg_param *pbbreg;
	u8 path;
	u8 offset;
	u32 value;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(struct rf_reg_param))
		return RNDIS_STATUS_INVALID_LENGTH;
	pbbreg = (struct rf_reg_param *)(poid_par_priv->information_buf);
	path = (u8)pbbreg->path;
	if (path > RF_PATH_B)
		return RNDIS_STATUS_NOT_ACCEPTED;
	offset = (u8)pbbreg->offset;
	value = pbbreg->value;
	r8712_rf_reg_write(Adapter, path, offset, value);
	return status;
}

uint oid_rt_pro_read_rf_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	struct rf_reg_param *pbbreg;
	u8 path;
	u8 offset;
	u32 value;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(struct rf_reg_param))
		return RNDIS_STATUS_INVALID_LENGTH;
	pbbreg = (struct rf_reg_param *)(poid_par_priv->information_buf);
	path = (u8)pbbreg->path;
	if (path > RF_PATH_B) /* 1T2R  path_a /path_b */
		return RNDIS_STATUS_NOT_ACCEPTED;
	offset = (u8)pbbreg->offset;
	value = r8712_rf_reg_read(Adapter, path, offset);
	pbbreg->value = value;
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}

/*This function initializes the DUT to the MP test mode*/
static int mp_start_test(struct _adapter *padapter)
{
	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *tgt_network = &pmlmepriv->cur_network;
	struct ndis_wlan_bssid_ex bssid;
	struct sta_info *psta;
	unsigned long length;
	unsigned long irqL;
	int res = _SUCCESS;

	/* 3 1. initialize a new struct ndis_wlan_bssid_ex */
	memcpy(bssid.MacAddress, pmppriv->network_macaddr, ETH_ALEN);
	bssid.Ssid.SsidLength = 16;
	memcpy(bssid.Ssid.Ssid, (unsigned char *)"mp_pseudo_adhoc",
		bssid.Ssid.SsidLength);
	bssid.InfrastructureMode = Ndis802_11IBSS;
	bssid.NetworkTypeInUse = Ndis802_11DS;
	bssid.IELength = 0;
	length = r8712_get_ndis_wlan_bssid_ex_sz(&bssid);
	if (length % 4) {
		/*round up to multiple of 4 bytes.*/
		bssid.Length = ((length >> 2) + 1) << 2;
	} else
		bssid.Length = length;
	spin_lock_irqsave(&pmlmepriv->lock, irqL);
	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == true)
		goto end_of_mp_start_test;
	/*init mp_start_test status*/
	pmppriv->prev_fw_state = get_fwstate(pmlmepriv);
	pmlmepriv->fw_state = WIFI_MP_STATE;
	if (pmppriv->mode == _LOOPBOOK_MODE_)
		set_fwstate(pmlmepriv, WIFI_MP_LPBK_STATE); /*append txdesc*/
	set_fwstate(pmlmepriv, _FW_UNDER_LINKING);
	/* 3 2. create a new psta for mp driver */
	/* clear psta in the cur_network, if any */
	psta = r8712_get_stainfo(&padapter->stapriv,
				 tgt_network->network.MacAddress);
	if (psta)
		r8712_free_stainfo(padapter, psta);
	psta = r8712_alloc_stainfo(&padapter->stapriv, bssid.MacAddress);
	if (psta == NULL) {
		res = _FAIL;
		goto end_of_mp_start_test;
	}
	/* 3 3. join psudo AdHoc */
	tgt_network->join_res = 1;
	tgt_network->aid = psta->aid = 1;
	memcpy(&tgt_network->network, &bssid, length);
	_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);
	r8712_os_indicate_connect(padapter);
	/* Set to LINKED STATE for MP TRX Testing */
	set_fwstate(pmlmepriv, _FW_LINKED);
end_of_mp_start_test:
	spin_unlock_irqrestore(&pmlmepriv->lock, irqL);
	return res;
}

/*This function change the DUT from the MP test mode into normal mode */
static int mp_stop_test(struct _adapter *padapter)
{
	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *tgt_network = &pmlmepriv->cur_network;
	struct sta_info *psta;
	unsigned long irqL;

	spin_lock_irqsave(&pmlmepriv->lock, irqL);
	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == false)
		goto end_of_mp_stop_test;
	/* 3 1. disconnect psudo AdHoc */
	r8712_os_indicate_disconnect(padapter);
	/* 3 2. clear psta used in mp test mode. */
	psta = r8712_get_stainfo(&padapter->stapriv,
				 tgt_network->network.MacAddress);
	if (psta)
		r8712_free_stainfo(padapter, psta);
	/* 3 3. return to normal state (default:station mode) */
	pmlmepriv->fw_state = pmppriv->prev_fw_state; /* WIFI_STATION_STATE;*/
	/*flush the cur_network*/
	memset(tgt_network, 0, sizeof(struct wlan_network));
end_of_mp_stop_test:
	spin_unlock_irqrestore(&pmlmepriv->lock, irqL);
	return _SUCCESS;
}

int mp_start_joinbss(struct _adapter *padapter, struct ndis_802_11_ssid *pssid)
{
	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	unsigned char res = _SUCCESS;

	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == false)
		return _FAIL;
	if (check_fwstate(pmlmepriv, _FW_LINKED) == false)
		return _FAIL;
	_clr_fwstate_(pmlmepriv, _FW_LINKED);
	res = r8712_setassocsta_cmd(padapter, pmppriv->network_macaddr);
	set_fwstate(pmlmepriv, _FW_UNDER_LINKING);
	return res;
}

uint oid_rt_pro_set_data_rate_hdl(struct oid_par_priv
					 *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u32 ratevalue;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len != sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	ratevalue = *((u32 *)poid_par_priv->information_buf);
	if (ratevalue >= MPT_RATE_LAST)
		return RNDIS_STATUS_INVALID_DATA;
	Adapter->mppriv.curr_rateidx = ratevalue;
	r8712_SetDataRate(Adapter);
	return status;
}

uint oid_rt_pro_start_test_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u32 mode;
	u8 val8;

	if (poid_par_priv->type_of_oid != SET_OID)
		return  RNDIS_STATUS_NOT_ACCEPTED;
	mode = *((u32 *)poid_par_priv->information_buf);
	Adapter->mppriv.mode = mode;/* 1 for loopback*/
	if (mp_start_test(Adapter) == _FAIL)
		status = RNDIS_STATUS_NOT_ACCEPTED;
	r8712_write8(Adapter, MSR, 1); /* Link in ad hoc network, 0x1025004C */
	r8712_write8(Adapter, RCR, 0); /* RCR : disable all pkt, 0x10250048 */
	/* RCR disable Check BSSID, 0x1025004a */
	r8712_write8(Adapter, RCR+2, 0x57);
	/* disable RX filter map , mgt frames will put in RX FIFO 0 */
	r8712_write16(Adapter, RXFLTMAP0, 0x0);
	val8 = r8712_read8(Adapter, EE_9346CR);
	if (!(val8 & _9356SEL)) { /*boot from EFUSE*/
		r8712_efuse_reg_init(Adapter);
		r8712_efuse_change_max_size(Adapter);
		r8712_efuse_reg_uninit(Adapter);
	}
	return status;
}

uint oid_rt_pro_stop_test_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (mp_stop_test(Adapter) == _FAIL)
		status = RNDIS_STATUS_NOT_ACCEPTED;
	return status;
}

uint oid_rt_pro_set_channel_direct_call_hdl(struct oid_par_priv
						   *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u32		Channel;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len != sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	Channel = *((u32 *)poid_par_priv->information_buf);
	if (Channel > 14)
		return RNDIS_STATUS_NOT_ACCEPTED;
	Adapter->mppriv.curr_ch = Channel;
	r8712_SetChannel(Adapter);
	return status;
}

uint oid_rt_pro_set_antenna_bb_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u32 antenna;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len != sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	antenna = *((u32 *)poid_par_priv->information_buf);
	Adapter->mppriv.antenna_tx = (u16)((antenna & 0xFFFF0000) >> 16);
	Adapter->mppriv.antenna_rx = (u16)(antenna & 0x0000FFFF);
	r8712_SwitchAntenna(Adapter);
	return status;
}

uint oid_rt_pro_set_tx_power_control_hdl(
					struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u32 tx_pwr_idx;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len != sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	tx_pwr_idx = *((u32 *)poid_par_priv->information_buf);
	if (tx_pwr_idx > MAX_TX_PWR_INDEX_N_MODE)
		return RNDIS_STATUS_NOT_ACCEPTED;
	Adapter->mppriv.curr_txpoweridx = (u8)tx_pwr_idx;
	r8712_SetTxPower(Adapter);
	return status;
}

uint oid_rt_pro_query_tx_packet_sent_hdl(
					struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID) {
		status = RNDIS_STATUS_NOT_ACCEPTED;
		return status;
	}
	if (poid_par_priv->information_buf_len == sizeof(u32)) {
		*(u32 *)poid_par_priv->information_buf =
					Adapter->mppriv.tx_pktcount;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	} else
		status = RNDIS_STATUS_INVALID_LENGTH;
	return status;
}

uint oid_rt_pro_query_rx_packet_received_hdl(
					struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID) {
		status = RNDIS_STATUS_NOT_ACCEPTED;
		return status;
	}
	if (poid_par_priv->information_buf_len == sizeof(u32)) {
		*(u32 *)poid_par_priv->information_buf =
					Adapter->mppriv.rx_pktcount;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	} else
		status = RNDIS_STATUS_INVALID_LENGTH;
	return status;
}

uint oid_rt_pro_query_rx_packet_crc32_error_hdl(
					struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID) {
		status = RNDIS_STATUS_NOT_ACCEPTED;
		return status;
	}
	if (poid_par_priv->information_buf_len == sizeof(u32)) {
		*(u32 *)poid_par_priv->information_buf =
					Adapter->mppriv.rx_crcerrpktcount;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	} else
		status = RNDIS_STATUS_INVALID_LENGTH;
	return status;
}

uint oid_rt_pro_reset_tx_packet_sent_hdl(struct oid_par_priv
						*poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	Adapter->mppriv.tx_pktcount = 0;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_reset_rx_packet_received_hdl(struct oid_par_priv
						    *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len == sizeof(u32)) {
		Adapter->mppriv.rx_pktcount = 0;
		Adapter->mppriv.rx_crcerrpktcount = 0;
	} else
		status = RNDIS_STATUS_INVALID_LENGTH;
	return status;
}

uint oid_rt_reset_phy_rx_packet_count_hdl(struct oid_par_priv
						 *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	r8712_ResetPhyRxPktCount(Adapter);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_get_phy_rx_packet_received_hdl(struct oid_par_priv
						  *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len != sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	*(u32 *)poid_par_priv->information_buf =
					 r8712_GetPhyRxPktReceived(Adapter);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_get_phy_rx_packet_crc32_error_hdl(struct oid_par_priv
						     *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len != sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	*(u32 *)poid_par_priv->information_buf =
					 r8712_GetPhyRxPktCRC32Error(Adapter);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_modulation_hdl(struct oid_par_priv
					  *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;

	Adapter->mppriv.curr_modem = *((u8 *)poid_par_priv->information_buf);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_continuous_tx_hdl(struct oid_par_priv
					     *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	u32		bStartTest;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	bStartTest = *((u32 *)poid_par_priv->information_buf);
	r8712_SetContinuousTx(Adapter, (u8)bStartTest);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_single_carrier_tx_hdl(struct oid_par_priv
						 *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	u32		bStartTest;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	bStartTest = *((u32 *)poid_par_priv->information_buf);
	r8712_SetSingleCarrierTx(Adapter, (u8)bStartTest);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_carrier_suppression_tx_hdl(struct oid_par_priv
						      *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	u32		bStartTest;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	bStartTest = *((u32 *)poid_par_priv->information_buf);
	r8712_SetCarrierSuppressionTx(Adapter, (u8)bStartTest);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_single_tone_tx_hdl(struct oid_par_priv
					      *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	u32		bStartTest;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	bStartTest = *((u32 *)poid_par_priv->information_buf);
	r8712_SetSingleToneTx(Adapter, (u8)bStartTest);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro8711_join_bss_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	struct ndis_802_11_ssid *pssid;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	*poid_par_priv->bytes_needed = (u32)sizeof(struct ndis_802_11_ssid);
	*poid_par_priv->bytes_rw = 0;
	if (poid_par_priv->information_buf_len < *poid_par_priv->bytes_needed)
		return RNDIS_STATUS_INVALID_LENGTH;
	pssid = (struct ndis_802_11_ssid *)poid_par_priv->information_buf;
	if (mp_start_joinbss(Adapter, pssid) == _FAIL)
		status = RNDIS_STATUS_NOT_ACCEPTED;
	*poid_par_priv->bytes_rw = sizeof(struct ndis_802_11_ssid);
	return status;
}

uint oid_rt_pro_read_register_hdl(struct oid_par_priv
					 *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	struct mp_rw_reg *RegRWStruct;
	u16		offset;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	RegRWStruct = (struct mp_rw_reg *)poid_par_priv->information_buf;
	if ((RegRWStruct->offset >= 0x10250800) &&
	    (RegRWStruct->offset <= 0x10250FFF)) {
		/*baseband register*/
		/*0ffset :0x800~0xfff*/
		offset = (u16)(RegRWStruct->offset) & 0xFFF;
		RegRWStruct->value = r8712_bb_reg_read(Adapter, offset);
	} else {
		switch (RegRWStruct->width) {
		case 1:
			RegRWStruct->value = r8712_read8(Adapter,
						   RegRWStruct->offset);
			break;
		case 2:
			RegRWStruct->value = r8712_read16(Adapter,
						    RegRWStruct->offset);
			break;
		case 4:
			RegRWStruct->value = r8712_read32(Adapter,
						    RegRWStruct->offset);
			break;
		default:
			status = RNDIS_STATUS_NOT_ACCEPTED;
			break;
		}
	}
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}

uint oid_rt_pro_write_register_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	struct mp_rw_reg *RegRWStruct;
	u16		offset;
	u32		value;
	u32 oldValue = 0;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	RegRWStruct = (struct mp_rw_reg *)poid_par_priv->information_buf;
	if ((RegRWStruct->offset >= 0x10250800) &&
	    (RegRWStruct->offset <= 0x10250FFF)) {
		/*baseband register*/
		offset = (u16)(RegRWStruct->offset) & 0xFFF;
		value = RegRWStruct->value;
		switch (RegRWStruct->width) {
		case 1:
			oldValue = r8712_bb_reg_read(Adapter, offset);
			oldValue &= 0xFFFFFF00;
			value &= 0x000000FF;
			value |= oldValue;
			break;
		case 2:
			oldValue = r8712_bb_reg_read(Adapter, offset);
			oldValue &= 0xFFFF0000;
			value &= 0x0000FFFF;
			value |= oldValue;
			break;
		}
		r8712_bb_reg_write(Adapter, offset, value);
	} else {
		switch (RegRWStruct->width) {
		case 1:
			r8712_write8(Adapter, RegRWStruct->offset,
			       (unsigned char)RegRWStruct->value);
			break;
		case 2:
			r8712_write16(Adapter, RegRWStruct->offset,
				(unsigned short)RegRWStruct->value);
			break;
		case 4:
			r8712_write32(Adapter, RegRWStruct->offset,
				(unsigned int)RegRWStruct->value);
			break;
		default:
			status = RNDIS_STATUS_NOT_ACCEPTED;
			break;
		}

		if ((status == RNDIS_STATUS_SUCCESS) &&
		    (RegRWStruct->offset == HIMR) &&
		    (RegRWStruct->width == 4))
			Adapter->ImrContent = RegRWStruct->value;
	}
	return status;
}

uint oid_rt_pro_burst_read_register_hdl(struct oid_par_priv
					       *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct burst_rw_reg *pBstRwReg;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	pBstRwReg = (struct burst_rw_reg *)poid_par_priv->information_buf;
	r8712_read_mem(Adapter, pBstRwReg->offset, (u32)pBstRwReg->len,
		 pBstRwReg->Data);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_burst_write_register_hdl(struct oid_par_priv
						*poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct burst_rw_reg *pBstRwReg;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	pBstRwReg = (struct burst_rw_reg *)poid_par_priv->information_buf;
	r8712_write_mem(Adapter, pBstRwReg->offset, (u32)pBstRwReg->len,
		  pBstRwReg->Data);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_write_txcmd_hdl(struct oid_par_priv *poid_par_priv)
{
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_read16_eeprom_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct eeprom_rw_param *pEEPROM;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	pEEPROM = (struct eeprom_rw_param *)poid_par_priv->information_buf;
	pEEPROM->value = r8712_eeprom_read16(Adapter,
					     (u16)(pEEPROM->offset >> 1));
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_write16_eeprom_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct eeprom_rw_param *pEEPROM;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	pEEPROM = (struct eeprom_rw_param *)poid_par_priv->information_buf;
	r8712_eeprom_write16(Adapter, (u16)(pEEPROM->offset >> 1),
			     pEEPROM->value);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro8711_wi_poll_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct mp_wiparam *pwi_param;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(struct mp_wiparam))
		return RNDIS_STATUS_INVALID_LENGTH;
	if (Adapter->mppriv.workparam.bcompleted == false)
		return RNDIS_STATUS_NOT_ACCEPTED;
	pwi_param = (struct mp_wiparam *)poid_par_priv->information_buf;
	memcpy(pwi_param, &Adapter->mppriv.workparam,
		sizeof(struct mp_wiparam));
	Adapter->mppriv.act_in_progress = false;
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro8711_pkt_loss_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(uint) * 2)
		return RNDIS_STATUS_INVALID_LENGTH;
	if (*(uint *)poid_par_priv->information_buf == 1)
		Adapter->mppriv.rx_pktloss = 0;
	*((uint *)poid_par_priv->information_buf+1) =
					 Adapter->mppriv.rx_pktloss;
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_rd_attrib_mem_hdl(struct oid_par_priv *poid_par_priv)
{
	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_wr_attrib_mem_hdl(struct oid_par_priv *poid_par_priv)
{
	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_rf_intfs_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (r8712_setrfintfs_cmd(Adapter, *(unsigned char *)
	    poid_par_priv->information_buf) == _FAIL)
		status = RNDIS_STATUS_NOT_ACCEPTED;
	return status;
}

uint oid_rt_poll_rx_status_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	memcpy(poid_par_priv->information_buf,
		(unsigned char *)&Adapter->mppriv.rxstat,
		sizeof(struct recv_stat));
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}

uint oid_rt_pro_cfg_debug_message_hdl(struct oid_par_priv
					     *poid_par_priv)
{
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_data_rate_ex_hdl(struct oid_par_priv
					    *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (r8712_setdatarate_cmd(Adapter,
	    poid_par_priv->information_buf) != _SUCCESS)
		status = RNDIS_STATUS_NOT_ACCEPTED;
	return status;
}

uint oid_rt_get_thermal_meter_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;

	if (Adapter->mppriv.act_in_progress == true)
		return RNDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(u8))
		return RNDIS_STATUS_INVALID_LENGTH;
	/*init workparam*/
	Adapter->mppriv.act_in_progress = true;
	Adapter->mppriv.workparam.bcompleted = false;
	Adapter->mppriv.workparam.act_type = MPT_GET_THERMAL_METER;
	Adapter->mppriv.workparam.io_offset = 0;
	Adapter->mppriv.workparam.io_value = 0xFFFFFFFF;
	r8712_GetThermalMeter(Adapter, &Adapter->mppriv.workparam.io_value);
	Adapter->mppriv.workparam.bcompleted = true;
	Adapter->mppriv.act_in_progress = false;
	*(u32 *)poid_par_priv->information_buf =
				 Adapter->mppriv.workparam.io_value;
	*poid_par_priv->bytes_rw = sizeof(u32);
	return status;
}

uint oid_rt_pro_set_power_tracking_hdl(struct oid_par_priv
					      *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(u8))
		return RNDIS_STATUS_INVALID_LENGTH;
	if (!r8712_setptm_cmd(Adapter, *((u8 *)poid_par_priv->information_buf)))
		status = RNDIS_STATUS_NOT_ACCEPTED;
	return status;
}

uint oid_rt_pro_set_basic_rate_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	u8 mpdatarate[NumRates] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0xff};
	uint status = RNDIS_STATUS_SUCCESS;
	u32 ratevalue;
	u8 datarates[NumRates];
	int i;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	ratevalue = *((u32 *)poid_par_priv->information_buf);
	for (i = 0; i < NumRates; i++) {
		if (ratevalue == mpdatarate[i])
			datarates[i] = mpdatarate[i];
		else
			datarates[i] = 0xff;
	}
	if (r8712_setbasicrate_cmd(Adapter, datarates) != _SUCCESS)
		status = RNDIS_STATUS_NOT_ACCEPTED;
	return status;
}

uint oid_rt_pro_qry_pwrstate_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < 8)
		return RNDIS_STATUS_INVALID_LENGTH;
	*poid_par_priv->bytes_rw = 8;
	memcpy(poid_par_priv->information_buf,
		&(Adapter->pwrctrlpriv.pwr_mode), 8);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_pwrstate_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint pwr_mode, smart_ps;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	*poid_par_priv->bytes_rw = 0;
	*poid_par_priv->bytes_needed = 8;
	if (poid_par_priv->information_buf_len < 8)
		return RNDIS_STATUS_INVALID_LENGTH;
	pwr_mode = *(uint *)(poid_par_priv->information_buf);
	smart_ps = *(uint *)((addr_t)poid_par_priv->information_buf + 4);
	if (pwr_mode != Adapter->pwrctrlpriv.pwr_mode || smart_ps !=
			Adapter->pwrctrlpriv.smart_ps)
		r8712_set_ps_mode(Adapter, pwr_mode, smart_ps);
	*poid_par_priv->bytes_rw = 8;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_h2c_set_rate_table_hdl(struct oid_par_priv
					      *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	struct setratable_parm *prate_table;
	u8 res;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	*poid_par_priv->bytes_needed  = sizeof(struct setratable_parm);
	if (poid_par_priv->information_buf_len <
	    sizeof(struct setratable_parm))
		return RNDIS_STATUS_INVALID_LENGTH;
	prate_table = (struct setratable_parm *)poid_par_priv->information_buf;
	res = r8712_setrttbl_cmd(Adapter, prate_table);
	if (res == _FAIL)
		status = RNDIS_STATUS_FAILURE;
	return status;
}

uint oid_rt_pro_h2c_get_rate_table_hdl(struct oid_par_priv
					      *poid_par_priv)
{
	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_encryption_ctrl_hdl(struct oid_par_priv
					   *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	struct security_priv *psecuritypriv = &Adapter->securitypriv;
	enum ENCRY_CTRL_STATE encry_mode = 0;

	*poid_par_priv->bytes_needed = sizeof(u8);
	if (poid_par_priv->information_buf_len < *poid_par_priv->bytes_needed)
		return RNDIS_STATUS_INVALID_LENGTH;

	if (poid_par_priv->type_of_oid == SET_OID) {
		encry_mode = *((u8 *)poid_par_priv->information_buf);
		switch (encry_mode) {
		case HW_CONTROL:
			psecuritypriv->sw_decrypt = false;
			psecuritypriv->sw_encrypt = false;
			break;
		case SW_CONTROL:
			psecuritypriv->sw_decrypt = true;
			psecuritypriv->sw_encrypt = true;
			break;
		case HW_ENCRY_SW_DECRY:
			psecuritypriv->sw_decrypt = true;
			psecuritypriv->sw_encrypt = false;
			break;
		case SW_ENCRY_HW_DECRY:
			psecuritypriv->sw_decrypt = false;
			psecuritypriv->sw_encrypt = true;
			break;
		}
	} else {
		if ((psecuritypriv->sw_encrypt == false) &&
		    (psecuritypriv->sw_decrypt == false))
			encry_mode = HW_CONTROL;
		else if ((psecuritypriv->sw_encrypt == false) &&
			 (psecuritypriv->sw_decrypt == true))
			encry_mode = HW_ENCRY_SW_DECRY;
		else if ((psecuritypriv->sw_encrypt == true) &&
			 (psecuritypriv->sw_decrypt == false))
			encry_mode = SW_ENCRY_HW_DECRY;
		else if ((psecuritypriv->sw_encrypt == true) &&
			 (psecuritypriv->sw_decrypt == true))
			encry_mode = SW_CONTROL;
		*(u8 *)poid_par_priv->information_buf =  encry_mode;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	}
	return RNDIS_STATUS_SUCCESS;
}
/*----------------------------------------------------------------------*/
uint oid_rt_pro_add_sta_info_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	uint status = RNDIS_STATUS_SUCCESS;

	struct sta_info	*psta = NULL;
	u8	*macaddr;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;

	*poid_par_priv->bytes_needed = ETH_ALEN;
	if (poid_par_priv->information_buf_len < *poid_par_priv->bytes_needed)
		return RNDIS_STATUS_INVALID_LENGTH;
	macaddr = (u8 *) poid_par_priv->information_buf;
	psta = r8712_get_stainfo(&Adapter->stapriv, macaddr);
	if (psta == NULL) { /* the sta in sta_info_queue => do nothing*/
		psta = r8712_alloc_stainfo(&Adapter->stapriv, macaddr);
		if (psta == NULL)
			status = RNDIS_STATUS_FAILURE;
	}
	return status;
}
/*-------------------------------------------------------------------------*/
uint oid_rt_pro_dele_sta_info_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	unsigned long			irqL;
	uint status = RNDIS_STATUS_SUCCESS;

	struct sta_info		*psta = NULL;
	u8			*macaddr;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;

	*poid_par_priv->bytes_needed = ETH_ALEN;
	if (poid_par_priv->information_buf_len < *poid_par_priv->bytes_needed)
		return RNDIS_STATUS_INVALID_LENGTH;

	macaddr = (u8 *)poid_par_priv->information_buf;

	psta = r8712_get_stainfo(&Adapter->stapriv, macaddr);
	if (psta != NULL) {
		spin_lock_irqsave(&(Adapter->stapriv.sta_hash_lock), irqL);
		r8712_free_stainfo(Adapter, psta);
		spin_unlock_irqrestore(&(Adapter->stapriv.sta_hash_lock), irqL);
	}

	return status;
}
/*--------------------------------------------------------------------------*/
static u32 mp_query_drv_var(struct _adapter *padapter, u8 offset, u32 var)
{
	return var;
}

uint oid_rt_pro_query_dr_variable_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	uint status = RNDIS_STATUS_SUCCESS;

	struct DR_VARIABLE_STRUCT *pdrv_var;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	*poid_par_priv->bytes_needed = sizeof(struct DR_VARIABLE_STRUCT);
	if (poid_par_priv->information_buf_len < *poid_par_priv->bytes_needed)
		return RNDIS_STATUS_INVALID_LENGTH;
	pdrv_var = (struct DR_VARIABLE_STRUCT *)poid_par_priv->information_buf;
	pdrv_var->variable = mp_query_drv_var(Adapter, pdrv_var->offset,
					      pdrv_var->variable);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}

/*--------------------------------------------------------------------------*/
uint oid_rt_pro_rx_packet_type_hdl(struct oid_par_priv *poid_par_priv)
{
	return RNDIS_STATUS_SUCCESS;
}
/*------------------------------------------------------------------------*/
uint oid_rt_pro_read_efuse_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	uint status = RNDIS_STATUS_SUCCESS;

	struct EFUSE_ACCESS_STRUCT *pefuse;
	u8 *data;
	u16 addr = 0, cnts = 0;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len <
	    sizeof(struct EFUSE_ACCESS_STRUCT))
		return RNDIS_STATUS_INVALID_LENGTH;
	pefuse = (struct EFUSE_ACCESS_STRUCT *)poid_par_priv->information_buf;
	addr = pefuse->start_addr;
	cnts = pefuse->cnts;
	data = pefuse->data;
	memset(data, 0xFF, cnts);
	if ((addr > 511) || (cnts < 1) || (cnts > 512) || (addr + cnts) >
	     EFUSE_MAX_SIZE)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (r8712_efuse_access(Adapter, true, addr, cnts, data) == false)
		status = RNDIS_STATUS_FAILURE;
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}
/*------------------------------------------------------------------------*/
uint oid_rt_pro_write_efuse_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	uint status = RNDIS_STATUS_SUCCESS;

	struct EFUSE_ACCESS_STRUCT *pefuse;
	u8 *data;
	u16 addr = 0, cnts = 0;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;

	pefuse = (struct EFUSE_ACCESS_STRUCT *)poid_par_priv->information_buf;
	addr = pefuse->start_addr;
	cnts = pefuse->cnts;
	data = pefuse->data;

	if ((addr > 511) || (cnts < 1) || (cnts > 512) ||
	    (addr + cnts) > r8712_efuse_get_max_size(Adapter))
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (r8712_efuse_access(Adapter, false, addr, cnts, data) == false)
		status = RNDIS_STATUS_FAILURE;
	return status;
}
/*----------------------------------------------------------------------*/
uint oid_rt_pro_rw_efuse_pgpkt_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	struct PGPKT_STRUCT	*ppgpkt;

	*poid_par_priv->bytes_rw = 0;
	if (poid_par_priv->information_buf_len < sizeof(struct PGPKT_STRUCT))
		return RNDIS_STATUS_INVALID_LENGTH;
	ppgpkt = (struct PGPKT_STRUCT *)poid_par_priv->information_buf;
	if (poid_par_priv->type_of_oid == QUERY_OID) {
		if (r8712_efuse_pg_packet_read(Adapter, ppgpkt->offset,
		    ppgpkt->data) == true)
			*poid_par_priv->bytes_rw =
				 poid_par_priv->information_buf_len;
		else
			status = RNDIS_STATUS_FAILURE;
	} else {
		if (r8712_efuse_reg_init(Adapter) == true) {
			if (r8712_efuse_pg_packet_write(Adapter, ppgpkt->offset,
			    ppgpkt->word_en, ppgpkt->data) == true)
				*poid_par_priv->bytes_rw =
					 poid_par_priv->information_buf_len;
			else
				status = RNDIS_STATUS_FAILURE;
			r8712_efuse_reg_uninit(Adapter);
		} else
			status = RNDIS_STATUS_FAILURE;
	}
	return status;
}

uint oid_rt_get_efuse_current_size_hdl(struct oid_par_priv
					      *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(int))
		return RNDIS_STATUS_INVALID_LENGTH;
	r8712_efuse_reg_init(Adapter);
	*(int *)poid_par_priv->information_buf =
				 r8712_efuse_get_current_size(Adapter);
	r8712_efuse_reg_uninit(Adapter);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}

uint oid_rt_get_efuse_max_size_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	*(int *)poid_par_priv->information_buf =
					 r8712_efuse_get_max_size(Adapter);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return status;
}

uint oid_rt_pro_efuse_hdl(struct oid_par_priv *poid_par_priv)
{
	uint status = RNDIS_STATUS_SUCCESS;

	if (poid_par_priv->type_of_oid == QUERY_OID)
		status = oid_rt_pro_read_efuse_hdl(poid_par_priv);
	else
		status = oid_rt_pro_write_efuse_hdl(poid_par_priv);
	return status;
}

uint oid_rt_pro_efuse_map_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u8		*data;

	*poid_par_priv->bytes_rw = 0;
	if (poid_par_priv->information_buf_len < EFUSE_MAP_MAX_SIZE)
		return RNDIS_STATUS_INVALID_LENGTH;
	data = (u8 *)poid_par_priv->information_buf;
	if (poid_par_priv->type_of_oid == QUERY_OID) {
		if (r8712_efuse_map_read(Adapter, 0, EFUSE_MAP_MAX_SIZE, data))
			*poid_par_priv->bytes_rw = EFUSE_MAP_MAX_SIZE;
		else
			status = RNDIS_STATUS_FAILURE;
	} else {
		/* SET_OID */
		if (r8712_efuse_reg_init(Adapter) == true) {
			if (r8712_efuse_map_write(Adapter, 0,
			    EFUSE_MAP_MAX_SIZE, data))
				*poid_par_priv->bytes_rw = EFUSE_MAP_MAX_SIZE;
			else
				status = RNDIS_STATUS_FAILURE;
			r8712_efuse_reg_uninit(Adapter);
		} else {
			status = RNDIS_STATUS_FAILURE;
		}
	}
	return status;
}

uint oid_rt_set_bandwidth_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u32		bandwidth;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	bandwidth = *((u32 *)poid_par_priv->information_buf);/*4*/
	if (bandwidth != HT_CHANNEL_WIDTH_20)
		bandwidth = HT_CHANNEL_WIDTH_40;
	Adapter->mppriv.curr_bandwidth = (u8)bandwidth;
	r8712_SwitchBandwidth(Adapter);
	return status;
}

uint oid_rt_set_crystal_cap_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	u32		crystal_cap = 0;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	crystal_cap = *((u32 *)poid_par_priv->information_buf);/*4*/
	if (crystal_cap > 0xf)
		return RNDIS_STATUS_NOT_ACCEPTED;
	Adapter->mppriv.curr_crystalcap = crystal_cap;
	r8712_SetCrystalCap(Adapter);
	return status;
}

uint oid_rt_set_rx_packet_type_hdl(struct oid_par_priv
					   *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	u8		rx_pkt_type;
	u32		rcr_val32;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(u8))
		return RNDIS_STATUS_INVALID_LENGTH;
	rx_pkt_type = *((u8 *)poid_par_priv->information_buf);/*4*/
	rcr_val32 = r8712_read32(Adapter, RCR);/*RCR = 0x10250048*/
	rcr_val32 &= ~(RCR_CBSSID | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);
	switch (rx_pkt_type) {
	case RX_PKT_BROADCAST:
		rcr_val32 |= (RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_ACRC32);
		break;
	case RX_PKT_DEST_ADDR:
		rcr_val32 |= (RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_ACRC32);
		break;
	case RX_PKT_PHY_MATCH:
		rcr_val32 |= (RCR_APM|RCR_ACRC32);
		break;
	default:
		rcr_val32 &= ~(RCR_AAP |
			       RCR_APM |
			       RCR_AM |
			       RCR_AB |
			       RCR_ACRC32);
		break;
	}
	if (rx_pkt_type == RX_PKT_DEST_ADDR)
		Adapter->mppriv.check_mp_pkt = 1;
	else
		Adapter->mppriv.check_mp_pkt = 0;
	r8712_write32(Adapter, RCR, rcr_val32);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_tx_agc_offset_hdl(struct oid_par_priv
					     *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	u32 txagc;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	txagc = *(u32 *)poid_par_priv->information_buf;
	r8712_SetTxAGCOffset(Adapter, txagc);
	return RNDIS_STATUS_SUCCESS;
}

uint oid_rt_pro_set_pkt_test_mode_hdl(struct oid_par_priv
					     *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);
	uint status = RNDIS_STATUS_SUCCESS;
	struct mlme_priv	*pmlmepriv = &Adapter->mlmepriv;
	struct mp_priv		*pmppriv = &Adapter->mppriv;
	u32			type;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;

	type = *(u32 *)poid_par_priv->information_buf;

	if (_LOOPBOOK_MODE_ == type) {
		pmppriv->mode = type;
		set_fwstate(pmlmepriv, WIFI_MP_LPBK_STATE); /*append txdesc*/
	} else if (_2MAC_MODE_ == type) {
		pmppriv->mode = type;
		_clr_fwstate_(pmlmepriv, WIFI_MP_LPBK_STATE);
	} else
		status = RNDIS_STATUS_NOT_ACCEPTED;
	return status;
}
/*--------------------------------------------------------------------------*/
/*Linux*/
unsigned int mp_ioctl_xmit_packet_hdl(struct oid_par_priv *poid_par_priv)
{
	return _SUCCESS;
}
/*-------------------------------------------------------------------------*/
uint oid_rt_set_power_down_hdl(struct oid_par_priv *poid_par_priv)
{
	u8	bpwrup;

	if (poid_par_priv->type_of_oid != SET_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	bpwrup = *(u8 *)poid_par_priv->information_buf;
	/*CALL  the power_down function*/
	return RNDIS_STATUS_SUCCESS;
}

/*-------------------------------------------------------------------------- */
uint oid_rt_get_power_mode_hdl(struct oid_par_priv *poid_par_priv)
{
	struct _adapter *Adapter = (struct _adapter *)
				   (poid_par_priv->adapter_context);

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return RNDIS_STATUS_NOT_ACCEPTED;
	if (poid_par_priv->information_buf_len < sizeof(u32))
		return RNDIS_STATUS_INVALID_LENGTH;
	*(int *)poid_par_priv->information_buf =
		 Adapter->registrypriv.low_power ? POWER_LOW : POWER_NORMAL;
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	return RNDIS_STATUS_SUCCESS;
}
