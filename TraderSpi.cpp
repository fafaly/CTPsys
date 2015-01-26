#include <windows.h>
#include <iostream>
using namespace std;

#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include "TraderSpi.h"

#pragma warning(disable : 4996)

// USER_API����
extern CThostFtdcTraderApi* pUserApi;

// ���ò���
extern char FRONT_ADDR[];		// ǰ�õ�ַ
extern char BROKER_ID[];		// ���͹�˾����
extern char INVESTOR_ID[];		// Ͷ���ߴ���
extern char PASSWORD[];			// �û�����
extern char INSTRUMENT_ID[];	// ��Լ����
extern TThostFtdcPriceType	LIMIT_PRICE;	// �۸�
extern TThostFtdcDirectionType	DIRECTION;	// ��������

// ������
extern int iRequestID;

// �Ự����
TThostFtdcFrontIDType	FRONT_ID;	//ǰ�ñ��
TThostFtdcSessionIDType	SESSION_ID;	//�Ự���
TThostFtdcOrderRefType	ORDER_REF;	//��������
TThostFtdcOrderRefType	EXECORDER_REF;	//ִ����������
TThostFtdcOrderRefType	FORQUOTE_REF;	//ѯ������
TThostFtdcOrderRefType	QUOTE_REF;	//��������

// �����ж�
bool IsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

void CTraderSpi::OnFrontConnected()
{
	cerr << "--->>> " << "OnFrontConnected" << endl;
	///�û���¼����
	ReqUserLogin();
}

void CTraderSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVESTOR_ID);
	strcpy(req.Password, PASSWORD);
	int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID);
	cerr << "--->>> �����û���¼����: " << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
}

void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspUserLogin" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// ����Ự����
		FRONT_ID = pRspUserLogin->FrontID;
		SESSION_ID = pRspUserLogin->SessionID;
		int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		iNextOrderRef++;
		sprintf(ORDER_REF, "%d", iNextOrderRef);
		sprintf(EXECORDER_REF, "%d", 1);
		sprintf(FORQUOTE_REF, "%d", 1);
		sprintf(QUOTE_REF, "%d", 1);
		///��ȡ��ǰ������
		cerr << "--->>> ��ȡ��ǰ������ = " << pUserApi->GetTradingDay() << endl;
		//ReqForQuoteInsert();//���۲�ѯ��֪�������õ�
		ReqOrderInsert();//����
		//ReqExecOrderInsert();
		//ReqQryInstrument();
		///Ͷ���߽�����ȷ��
		ReqSettlementInfoConfirm();
		//ReqQryTradingAccount();

	}
}

void CTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	int iResult = pUserApi->ReqSettlementInfoConfirm(&req, ++iRequestID);
	cerr << "--->>> Ͷ���߽�����ȷ��: " << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
}

void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///�����ѯ��Լ
		ReqQryInstrument();
	}
}

void CTraderSpi::ReqQryInstrument()
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	while (true)
	{
		int iResult = pUserApi->ReqQryInstrument(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << "--->>> �����ѯ��Լ: "  << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
			break;
		}
		else
		{
			cerr << "--->>> �����ѯ��Լ: "  << iResult << ", �ܵ�����" << endl;
			Sleep(1000);
		}
	} // while
}

void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryInstrument" << endl;
	cout << pInstrument->ExchangeInstID << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///�����ѯ��Լ
		ReqQryTradingAccount();
	}
}

void CTraderSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	while (true)
	{
		int iResult = pUserApi->ReqQryTradingAccount(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << "--->>> �����ѯ�ʽ��˻�: "  << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
			break;
		}
		else
		{
			cerr << "--->>> �����ѯ�ʽ��˻�: "  << iResult << ", �ܵ�����" << endl;
			Sleep(1000);
		}
	} // while
}

void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryTradingAccount" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		cout << "�ϴ���Ѻ��� " << pTradingAccount->PreMortgage << endl;
		cout << "�ϴ����ö�� " << pTradingAccount->PreCredit << endl;
		cout << "�ϴδ��� " << pTradingAccount->PreDeposit << endl;
		cout << "�ϴν���׼���� " << pTradingAccount->PreBalance << endl;
		cout << "�ϴ�ռ�õı�֤�� " << pTradingAccount->PreMargin << endl;
		cout << "��Ϣ���� " << pTradingAccount->InterestBase << endl;
		cout << "��Ϣ���� " << pTradingAccount->Interest << endl;
		cout << "����� " << pTradingAccount->Deposit << endl;
		///�����ѯͶ���ֲ߳�
		ReqQryInvestorPosition();
	}
}

void CTraderSpi::ReqQryInvestorPosition()
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	while (true)
	{
		int iResult = pUserApi->ReqQryInvestorPosition(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << "--->>> �����ѯͶ���ֲ߳�: "  << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
			break;
		}
		else
		{
			cerr << "--->>> �����ѯͶ���ֲ߳�: "  << iResult << ", �ܵ�����" << endl;
			Sleep(1000);
		}
	} // while
}

void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryInvestorPosition" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///����¼������
		ReqOrderInsert();
		//ִ������¼������
		ReqExecOrderInsert();
		//ѯ��¼��
		ReqForQuoteInsert();
		//�����̱���¼��
		ReqQuoteInsert();
	}
}

void CTraderSpi::ReqOrderInsert()
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, BROKER_ID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, INVESTOR_ID);
	///��Լ����
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	///��������
	strcpy(req.OrderRef, ORDER_REF);
	///�û�����
//	TThostFtdcUserIDType	UserID;
	///�����۸�����: �޼�
	req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
	///��������: 
	req.Direction = DIRECTION;
	///��Ͽ�ƽ��־: ����
	req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///���Ͷ���ױ���־
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///�۸�
	req.LimitPrice = 0;//�м۵�ֱ��д0���� THOST_FTDC_OPT_AskPrice1;//��һ��
	///����: 1
	req.VolumeTotalOriginal = 1;
	///��Ч������: ������Ч
	req.TimeCondition = THOST_FTDC_TC_GFD;
	///GTD����
//	TThostFtdcDateType	GTDDate;
	///�ɽ�������: �κ�����
	req.VolumeCondition = THOST_FTDC_VC_AV;
	///��С�ɽ���: 1
	req.MinVolume = 1;
	///��������: ����
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	///ֹ���
//	TThostFtdcPriceType	StopPrice;
	///ǿƽԭ��: ��ǿƽ
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///�Զ������־: ��
	req.IsAutoSuspend = 0;
	///ҵ��Ԫ
//	TThostFtdcBusinessUnitType	BusinessUnit;
	///������
//	TThostFtdcRequestIDType	RequestID;
	///�û�ǿ����־: ��
	req.UserForceClose = 0;

	int iResult = pUserApi->ReqOrderInsert(&req, ++iRequestID);
	cerr << "--->>> ����¼������: " << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
}

//ִ������¼������
void CTraderSpi::ReqExecOrderInsert()
{
	CThostFtdcInputExecOrderField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, BROKER_ID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, INVESTOR_ID);
	///��Լ����
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	///��������
	strcpy(req.ExecOrderRef, EXECORDER_REF);
	///�û�����
	//	TThostFtdcUserIDType	UserID;
	///����
	req.Volume=1;
	///������
	//TThostFtdcRequestIDType	RequestID;
	///ҵ��Ԫ
	//TThostFtdcBusinessUnitType	BusinessUnit;
	///��ƽ��־
	req.OffsetFlag=THOST_FTDC_OF_Close;//���������������Ҫ��ƽ���ƽ��
	///Ͷ���ױ���־
	req.HedgeFlag=THOST_FTDC_HF_Speculation;
	///ִ������
	req.ActionType=THOST_FTDC_ACTP_Exec;//�������ִ������THOST_FTDC_ACTP_Abandon
	///����ͷ������ĳֲַ���
	req.PosiDirection=THOST_FTDC_PD_Long;
	///��Ȩ��Ȩ���Ƿ����ڻ�ͷ��ı��
	req.ReservePositionFlag=THOST_FTDC_EOPF_UnReserve;//�����н��������������֣������THOST_FTDC_EOPF_Reserve
	///��Ȩ��Ȩ�����ɵ�ͷ���Ƿ��Զ�ƽ��
	req.CloseFlag=THOST_FTDC_EOCF_AutoClose;//�����н��������������֣������THOST_FTDC_EOCF_NotToClose

	int iResult = pUserApi->ReqExecOrderInsert(&req, ++iRequestID);
	cerr << "--->>> ִ������¼������: " << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
}

//ѯ��¼������
void CTraderSpi::ReqForQuoteInsert()
{
	CThostFtdcInputForQuoteField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, BROKER_ID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, INVESTOR_ID);
	///��Լ����
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	///��������
	strcpy(req.ForQuoteRef, EXECORDER_REF);
	///�û�����
	//	TThostFtdcUserIDType	UserID;

	int iResult = pUserApi->ReqForQuoteInsert(&req, ++iRequestID);
	cerr << "--->>> ѯ��¼������: " << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
}
//����¼������
void CTraderSpi::ReqQuoteInsert()
{
	CThostFtdcInputQuoteField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, BROKER_ID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, INVESTOR_ID);
	///��Լ����
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	///��������
	strcpy(req.QuoteRef, QUOTE_REF);
	///���۸�
	req.AskPrice=LIMIT_PRICE;
	///��۸�
	req.BidPrice=LIMIT_PRICE-1.0;
	///������
	req.AskVolume=1;
	///������
	req.BidVolume=1;
	///������
	//TThostFtdcRequestIDType	RequestID;
	///ҵ��Ԫ
	//TThostFtdcBusinessUnitType	BusinessUnit;
	///����ƽ��־
	req.AskOffsetFlag=THOST_FTDC_OF_Open;
	///��ƽ��־
	req.BidOffsetFlag=THOST_FTDC_OF_Open;
	///��Ͷ���ױ���־
	req.AskHedgeFlag=THOST_FTDC_HF_Speculation;
	///��Ͷ���ױ���־
	req.BidHedgeFlag=THOST_FTDC_HF_Speculation;
	
	int iResult = pUserApi->ReqQuoteInsert(&req, ++iRequestID);
	cerr << "--->>> ����¼������: " << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
}

void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderInsert" << endl;
	if (!IsErrorRspInfo(pRspInfo))
	{
		
	}

}

void CTraderSpi::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//���ִ��������ȷ���򲻻����ûص�
	cerr << "--->>> " << "OnRspExecOrderInsert" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//���ѯ����ȷ���򲻻����ûص�
	cerr << "--->>> " << "OnRspForQuoteInsert" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//���������ȷ���򲻻����ûص�
	cerr << "--->>> " << "OnRspQuoteInsert" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::ReqOrderAction(CThostFtdcOrderField *pOrder)
{
	static bool ORDER_ACTION_SENT = false;		//�Ƿ����˱���
	if (ORDER_ACTION_SENT)
		return;

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, pOrder->BrokerID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, pOrder->InvestorID);
	///������������
//	TThostFtdcOrderActionRefType	OrderActionRef;
	///��������
	strcpy(req.OrderRef, pOrder->OrderRef);
	///������
//	TThostFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	req.FrontID = FRONT_ID;
	///�Ự���
	req.SessionID = SESSION_ID;
	///����������
//	TThostFtdcExchangeIDType	ExchangeID;
	///�������
//	TThostFtdcOrderSysIDType	OrderSysID;
	///������־
	req.ActionFlag = THOST_FTDC_AF_Delete;
	///�۸�
//	TThostFtdcPriceType	LimitPrice;
	///�����仯
//	TThostFtdcVolumeType	VolumeChange;
	///�û�����
//	TThostFtdcUserIDType	UserID;
	///��Լ����
	strcpy(req.InstrumentID, pOrder->InstrumentID);

	int iResult = pUserApi->ReqOrderAction(&req, ++iRequestID);
	cerr << "--->>> ������������: "  << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
	ORDER_ACTION_SENT = true;
}

void CTraderSpi::ReqExecOrderAction(CThostFtdcExecOrderField *pExecOrder)
{
	static bool EXECORDER_ACTION_SENT = false;		//�Ƿ����˱���
	if (EXECORDER_ACTION_SENT)
		return;

	CThostFtdcInputExecOrderActionField req;
	memset(&req, 0, sizeof(req));

	///���͹�˾����
	strcpy(req.BrokerID,pExecOrder->BrokerID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID,pExecOrder->InvestorID);
	///ִ�������������
	//TThostFtdcOrderActionRefType	ExecOrderActionRef;
	///ִ����������
	strcpy(req.ExecOrderRef,pExecOrder->ExecOrderRef);
	///������
	//TThostFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	req.FrontID=FRONT_ID;
	///�Ự���
	req.SessionID=SESSION_ID;
	///����������
	//TThostFtdcExchangeIDType	ExchangeID;
	///ִ������������
	//TThostFtdcExecOrderSysIDType	ExecOrderSysID;
	///������־
	req.ActionFlag=THOST_FTDC_AF_Delete;
	///�û�����
	//TThostFtdcUserIDType	UserID;
	///��Լ����
	strcpy(req.InstrumentID,pExecOrder->InstrumentID);

	int iResult = pUserApi->ReqExecOrderAction(&req, ++iRequestID);
	cerr << "--->>> ִ�������������: "  << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
	EXECORDER_ACTION_SENT = true;
}

void CTraderSpi::ReqQuoteAction(CThostFtdcQuoteField *pQuote)
{
	static bool QUOTE_ACTION_SENT = false;		//�Ƿ����˱���
	if (QUOTE_ACTION_SENT)
		return;

	CThostFtdcInputQuoteActionField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, pQuote->BrokerID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, pQuote->InvestorID);
	///���۲�������
	//TThostFtdcOrderActionRefType	QuoteActionRef;
	///��������
	strcpy(req.QuoteRef,pQuote->QuoteRef);
	///������
	//TThostFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	req.FrontID=FRONT_ID;
	///�Ự���
	req.SessionID=SESSION_ID;
	///����������
	//TThostFtdcExchangeIDType	ExchangeID;
	///���۲������
	//TThostFtdcOrderSysIDType	QuoteSysID;
	///������־
	req.ActionFlag=THOST_FTDC_AF_Delete;
	///�û�����
	//TThostFtdcUserIDType	UserID;
	///��Լ����
	strcpy(req.InstrumentID,pQuote->InstrumentID);

	int iResult = pUserApi->ReqQuoteAction(&req, ++iRequestID);
	cerr << "--->>> ���۲�������: "  << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
	QUOTE_ACTION_SENT = true;
}

void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderAction" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInpuExectOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//��ȷ�ĳ����������������ûص�
	cerr << "--->>> " << "OnRspExecOrderAction" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInpuQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//��ȷ�ĳ����������������ûص�
	cerr << "--->>> " << "OnRspQuoteAction" << endl;
	IsErrorRspInfo(pRspInfo);
}

///����֪ͨ
void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	cerr << "--->>> " << "OnRtnOrder"  << endl;
	if (IsMyOrder(pOrder))
	{
		if (IsTradingOrder(pOrder))
			ReqOrderAction(pOrder);
		else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
			cout << "--->>> �����ɹ�" << endl;
	}
}

//ִ������֪ͨ
void CTraderSpi::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
	cerr << "--->>> " << "OnRtnExecOrder"  << endl;
	if (IsMyExecOrder(pExecOrder))
	{
		if (IsTradingExecOrder(pExecOrder))
			ReqExecOrderAction(pExecOrder);
		else if (pExecOrder->ExecResult == THOST_FTDC_OER_Canceled)
			cout << "--->>> ִ�����泷���ɹ�" << endl;
	}
}

//ѯ��֪ͨ
void CTraderSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
	//�������н���ѯ��֪ͨͨ���ýӿڷ��أ�ֻ�������̿ͻ������յ���֪ͨ
	cerr << "--->>> " << "OnRtnForQuoteRsp"  << endl;
}

//����֪ͨ
void CTraderSpi::OnRtnQuote(CThostFtdcQuoteField *pQuote)
{
	cerr << "--->>> " << "OnRtnQuote"  << endl;
	if (IsMyQuote(pQuote))
	{
		if (IsTradingQuote(pQuote))
			ReqQuoteAction(pQuote);
		else if (pQuote->QuoteStatus == THOST_FTDC_OST_Canceled)
			cout << "--->>> ���۳����ɹ�" << endl;
	}
}

///�ɽ�֪ͨ
void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	cerr << "--->>> " << "OnRtnTrade"  << endl;
	cout << "�ɽ�����" << pTrade->Volume << endl;
	cout << "�ɽ�ʱ�䣺" << pTrade->TradeTime << endl;
}

void CTraderSpi:: OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << "OnFrontDisconnected" << endl;
	cerr << "--->>> Reason = " << nReason << endl;
}
		
void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << "OnHeartBeatWarning" << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspError" << endl;
	IsErrorRspInfo(pRspInfo);
}

bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
}

bool CTraderSpi::IsMyOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->FrontID == FRONT_ID) &&
			(pOrder->SessionID == SESSION_ID) &&
			(strcmp(pOrder->OrderRef, ORDER_REF) == 0));
}

bool CTraderSpi::IsMyExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
	return ((pExecOrder->FrontID == FRONT_ID) &&
		(pExecOrder->SessionID == SESSION_ID) &&
		(strcmp(pExecOrder->ExecOrderRef, EXECORDER_REF) == 0));
}

bool CTraderSpi::IsMyQuote(CThostFtdcQuoteField *pQuote)
{
	return ((pQuote->FrontID == FRONT_ID) &&
		(pQuote->SessionID == SESSION_ID) &&
		(strcmp(pQuote->QuoteRef, QUOTE_REF) == 0));
}

bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

bool CTraderSpi::IsTradingExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
	return (pExecOrder->ExecResult != THOST_FTDC_OER_Canceled);
}

bool CTraderSpi::IsTradingQuote(CThostFtdcQuoteField *pQuote)
{
	return (pQuote->QuoteStatus != THOST_FTDC_OST_Canceled);
}
