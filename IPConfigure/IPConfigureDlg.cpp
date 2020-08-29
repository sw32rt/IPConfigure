
// IPConfigureDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "IPConfigure.h"
#include "IPConfigureDlg.h"
#include "afxdialogex.h"
#include <WS2tcpip.h>
#pragma comment( lib, "iphlpapi.lib" )


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_ICON					(100)
#define WM_USER_TRAYNOTIFYICON	(WM_APP+100)
#define ID_CLICKTIMER			(4)
#define ID_TMR_WINDOWOPEN           (11)
#define ID_TMR_WINDOWCLOSE_DELAY    (12)
#define ID_TMR_WINDOWCLOSE          (13)

// CIPConfigureDlg ダイアログ



CIPConfigureDlg::CIPConfigureDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IPCONFIGURE_DIALOG, pParent)
	, m_hIcon_main(NULL)
	, m_hIcon_current(NULL)
	, m_bFireDoubleClick(false)
	, m_WindowOpenTimerID(0)
	, m_WindowCloseDelayTimerID(0)
	, m_WindowCloseTimerID(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIPConfigureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIPConfigureDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CIPConfigureDlg::OnBnClickedOk)
	ON_MESSAGE(WM_USER_TRAYNOTIFYICON, OnTrayNotify)
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CIPConfigureDlg メッセージ ハンドラー

BOOL CIPConfigureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定


	// TODO: 初期化をここに追加します。
	CRect rect;
	HWND hDeskWnd = ::GetDesktopWindow(); //この関数でデスクトップのハンドルを取得
    ::GetWindowRect(hDeskWnd, rect); //デスクトップのハンドルからその(画面の)大きさを取得
	rect.bottom -= 60;
	rect.top = rect.bottom - WINDOWSIZE_FIXHEIGHT;
	rect.left = rect.right - WINDOWSIZE_COLLAPSE;
	MoveWindow(rect, TRUE);
	
	// 現在のウィンドウスタイルを取得
	UINT style = ::GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE);
	// ウィンドウスタイルにWS_EX_LAYEREDを追加
	style |= WS_EX_LAYERED;
	// ウィンドウにスタイル適用
	::SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, style);
	// ウィンドウの透明度を設定(127が透明度、0〜255で指定)
	this->SetLayeredWindowAttributes(0, 127, LWA_ALPHA);

	// ウィンドウ背景色のブラシを作成する．
	m_brDlg.CreateSolidBrush(RGB(0, 0, 0));

	UpdateIPAddress();
	
	/* IPaddress変更監視スレッド生成 */
	AfxBeginThread(IPAddressChangeWatchThread, this);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CIPConfigureDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CClientDC cdc(this);
		CFont* pFont;
		CFont font;
		CRect rect;

		CPoint startPoint;
		CPoint endPoint;

		cdc.SetTextColor(RGB(255, 255, 255));
		cdc.SetBkMode(TRANSPARENT);

		// フォントを作成する
		font.CreateFont(
			16,                   // フォントの高さ(大きさ)。
			0,                    // フォントの幅。普通０。
			0,                    // 角度。０でＯＫ。
			0,                    // 同じく角度。これも０。
			FW_DONTCARE,          // 文字の太さ。
			FALSE,                // フォントがイタリックならTRUEを指定。
			FALSE,                // 下線を引くならTRUE。
			FALSE,                // 取り消し線を引くならTRUE。
			SHIFTJIS_CHARSET,     // フォントの文字セット。このままでＯＫ。
			OUT_DEFAULT_PRECIS,   // 出力精度の設定。このままでＯＫ。
			CLIP_DEFAULT_PRECIS,  // クリッピング精度。このままでＯＫ。
			DRAFT_QUALITY,        // フォントの出力品質。このままでＯＫ。
			//DEFAULT_PITCH,        // フォントのピッチとファミリを指定。このままでＯＫ。
			FIXED_PITCH,        // フォントのピッチとファミリを指定。このままでＯＫ。
			_T("Consolas") // フォントのタイプフェイス名の指定。これは見たまんま。
			//_T("Meiryo UI") // フォントのタイプフェイス名の指定。これは見たまんま。
		);
		pFont = cdc.SelectObject(&font);        // フォントを設定。

		unsigned short ushTextOutYPosition = 0;
		CSize TextSize = cdc.GetTextExtent(L"A");
		for(CString str : m_DisplayStringVector)
		{
			cdc.TextOut(10, TextSize.cy * ushTextOutYPosition, str);
			ushTextOutYPosition++;
		}

		cdc.SelectObject(pFont);                // フォントを元に戻す。

		CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
		CPen* pOldPen = cdc.SelectObject(&pen);

		GetClientRect(rect);
		startPoint.x = 0;
		startPoint.y = (rect.Height() / 2) - 15;
		endPoint.x = 0;
		endPoint.y = (rect.Height() / 2) + 15;
		cdc.MoveTo(startPoint);
		cdc.LineTo(endPoint);

		//startPoint.x += 3;
		//endPoint.x += 3;
		//cdc.MoveTo(startPoint);
		//cdc.LineTo(endPoint);

		//startPoint.x += 3;
		//endPoint.x += 3;
		//cdc.MoveTo(startPoint);
		//cdc.LineTo(endPoint);

		cdc.SelectObject(pOldPen);
		pen.DeleteObject();

		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CIPConfigureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



int CIPConfigureDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: ここに特定な作成コードを追加してください。
	m_hIcon_main = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	NotifyIcon(NIM_ADD, m_hIcon_main, _T("IPConfigure"));

	return 0;
}


void CIPConfigureDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
	NotifyIcon(NIM_DELETE, NULL);
}

BOOL CIPConfigureDlg::NotifyIcon(DWORD dwMessage, HICON hIcon, LPCTSTR pszTip /*= NULL*/)
{
	ASSERT(NIM_ADD == dwMessage
		|| NIM_DELETE == dwMessage
		|| NIM_MODIFY == dwMessage);

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = GetSafeHwnd();
	nid.uID = ID_ICON;	// この関数は、アイコンを1つだけサポートする。
										// タスクトレイのアイコンID
										// 1つのプロセスで複数のアイコンをタスクトレイに表示する場合には、それぞれ異なるIDを指定する。
										// このIDはプロセスごとに独立しているので、他のプロセスが同じIDを指定していたとしても、それらが衝突することはない。
	nid.uCallbackMessage = WM_USER_TRAYNOTIFYICON;
	nid.uFlags = NIF_MESSAGE;

	if (NULL != hIcon
		&& m_hIcon_current != hIcon)
	{	// アイコン変更
		nid.uFlags |= NIF_ICON;
		nid.hIcon = hIcon;
		m_hIcon_current = hIcon;
	}

	if (NULL != pszTip)
	{	// ツールチップ表示
		nid.uFlags |= NIF_TIP;
		_tcsncpy_s(nid.szTip, _countof(nid.szTip), pszTip, _TRUNCATE);
	}

	return Shell_NotifyIcon(dwMessage, &nid);
}

BOOL CIPConfigureDlg::NotifyIcon(DWORD dwMessage, HICON hIcon, UINT nStringResource)
{
	// 表示メッセージを、ストリングリソースのリソースＩＤで指定する関数
	CString msg;
	VERIFY(msg.LoadString(nStringResource));
	return NotifyIcon(dwMessage, hIcon, msg);
}

LRESULT CIPConfigureDlg::OnTrayNotify(UINT wParam, LONG lParam)
{
	UINT uiIconID = (UINT)wParam;
	UINT uiMouseMsg = (UINT)lParam;

	if (ID_ICON != uiIconID)
	{
		return 0;
	}

	// ここで模造メッセージをポストする理由：
	// この通知関数からは、できるだけ早く抜け出す必要がある。
	// たとえば、OnLButtonDownでOLEコントロールを作成しようとすると、
	// 暗号のようなエラーコードRPC_E_CANTCALLOUT_ININPUTSYNCCALLを
	// MFCから受け取ることになるだろう。
	// WinError.hによるとこのエラーの意味は「アプリケーションが入力
	// 同期呼び出しをディスパッチしているので、発信呼び出しはできない。」

	switch (uiMouseMsg)
	{
	case WM_LBUTTONDOWN:
		m_bFireDoubleClick = FALSE;
		SetTimer(ID_CLICKTIMER, GetDoubleClickTime(), NULL);
		break;

	case WM_LBUTTONUP:
		if (m_bFireDoubleClick)
		{
			PostMessage(WM_LBUTTONDBLCLK);
		}
		break;

	case WM_LBUTTONDBLCLK:
		m_bFireDoubleClick = TRUE;
		KillTimer(ID_CLICKTIMER);
		break;

	case WM_RBUTTONUP:
		PostMessage(WM_RBUTTONUP);
		break;
	}

	return 0;
}


HBRUSH CIPConfigureDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor) {
	case CTLCOLOR_DLG:
		return (HBRUSH)m_brDlg;
	default:
		break;
	}

	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: ここで DC の属性を変更してください。
	
	// TODO: 既定値を使用したくない場合は別のブラシを返します。
	return hbr;
}


void CIPConfigureDlg::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	//CDialogEx::OnOK();
}


void CIPConfigureDlg::UpdateIPAddress()
{
	PIP_ADAPTER_ADDRESSES pAdapterAddresses = { 0 };
	PIP_ADAPTER_ADDRESSES pAA = { 0 };
	DWORD dwRet = 0;
	DWORD dwSize = 0;
	char szAdapterName[BUFSIZ] = { 0 };
	int len = 0;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = nullptr;
	sockaddr* pAddr = nullptr;
	wchar_t wstrAddr[NI_MAXHOST] = { 0 };
	CString strDisplayStringTemp;

	/* 一旦クライアント領域を消す。(背景で塗りつぶす。)TextOutが残るため。 */
	InvalidateRect(NULL, TRUE);

	// ネットワークアダプタリストの一覧を格納するために
	// 必要なバッファサイズを取得する
	dwRet = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &dwSize);
	if (dwRet != ERROR_BUFFER_OVERFLOW) {
		exit(1);
	}

	// 一覧を格納する領域を確保
	pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(dwSize);
	if (pAdapterAddresses == NULL) {
		exit(1);
	}

	// アダプタリストの取得
	dwRet = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX,
		NULL, pAdapterAddresses,
		&dwSize);
	if (dwRet != ERROR_SUCCESS) {
		exit(1);
	}

	m_DisplayStringVector.clear();
	// アダプタ一覧を出力
	for (pAA = pAdapterAddresses; pAA; pAA = pAA->Next) {

		//ネットワーク接続の種類を確認
		if (IF_TYPE_SOFTWARE_LOOPBACK == pAA->IfType)
		{ /* ループバックアドレスは表示しない */
			continue;
		}

		/* ネットワークアダプタのフレンドリ名 */
		strDisplayStringTemp.Format(L"%s\n", pAA->FriendlyName);
		m_DisplayStringVector.push_back(strDisplayStringTemp);

		pUnicast = pAA->FirstUnicastAddress;            // ユニキャストアドレスリストの先頭アドレスをセット
		while (pUnicast)
		{
			// ユニキャストIPアドレスを列挙
			pAddr = pUnicast->Address.lpSockaddr;
			if (pAddr->sa_family == AF_INET)            // IPv4
			{
				InetNtop(AF_INET, &((struct sockaddr_in*)pAddr)->sin_addr, wstrAddr, NI_MAXHOST);
				strDisplayStringTemp.Format(L"　🏠IPv4 : %s / %d\n", wstrAddr, pUnicast->OnLinkPrefixLength);
				m_DisplayStringVector.push_back(strDisplayStringTemp);
			}
#if 0
			else if (pAddr->sa_family == AF_INET6)      // IPv6
			{
				InetNtop(AF_INET6, &((struct sockaddr_in6*)pAddr)->sin6_addr, wstrAddr, NI_MAXHOST);
				strDisplayStringTemp.Format(L"∹IPv6 : %s / %d\n", wstrAddr, pUnicast->OnLinkPrefixLength);
				m_DisplayStringVector.push_back(strDisplayStringTemp);
			}
#endif
			pUnicast = pUnicast->Next;                  // 次のユニキャストアドレスへ
		}
		strDisplayStringTemp = L"";
		m_DisplayStringVector.push_back(strDisplayStringTemp);
	}

	// メモリの解放
	free(pAdapterAddresses);
	/* 再描画させる。領域を無効化しOnPaint()を呼ばせる。NULLを指定するとクライアント領域全体を無効化する。*/
	/* 第二引数をFALSEにすると背景を再描画しない。TRUEにすると背景が再描画されTextOutしたものが消えてしまう。 */
	InvalidateRect(NULL, FALSE);

	return;
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
}

UINT CIPConfigureDlg::IPAddressChangeWatchThread(LPVOID p)
{
	// IPアドレス変更イベントハンドラ
	CIPConfigureDlg* pDlg = (CIPConfigureDlg*)p;
	while (1)
	{
		OVERLAPPED overlap;
		DWORD ret;

		HANDLE hand = WSACreateEvent();
		overlap.hEvent = WSACreateEvent();

		ret = NotifyAddrChange(&hand, &overlap);

		if (ret != NO_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("NotifyAddrChange error...%d\n", WSAGetLastError());
				return 1;
			}
		}

		if (WaitForSingleObject(overlap.hEvent, INFINITE) == WAIT_OBJECT_0) {
			printf("IP Address table changed.\n");
		}

		pDlg->OnIPAddressChanged();
	}
}

void CIPConfigureDlg::OnIPAddressChanged()
{
	UpdateIPAddress();
}


void CIPConfigureDlg::OnTimer(UINT_PTR nIDEvent)
{
	UINT_PTR timerID;
	UINT interval;

	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	if (ID_CLICKTIMER == nIDEvent)
	{
		KillTimer(ID_CLICKTIMER);
		PostMessage(WM_LBUTTONUP);
	}

	if (ID_TMR_WINDOWOPEN == nIDEvent)
	{
		CRect windowRect;
		GetWindowRect(windowRect);
		LONG64 width = windowRect.Width();

		if (width >= WINDOWSIZE_EXPAND)
		{
			StartTimer(ID_TMR_WINDOWOPEN);
			return;
		}
		
		windowRect.left -= ((WINDOWSIZE_EXPAND - width) * WINDOWSIZE_RATE) + 1;
		MoveWindow(windowRect, TRUE);
	}

	if (ID_TMR_WINDOWCLOSE_DELAY == nIDEvent)
	{
		StopTimer(ID_TMR_WINDOWOPEN);
		StopTimer(ID_TMR_WINDOWCLOSE_DELAY);
		StartTimer(ID_TMR_WINDOWCLOSE);
	}

	if (ID_TMR_WINDOWCLOSE == nIDEvent)
	{
		CRect windowRect;
		GetWindowRect(windowRect);
		LONG64 width = windowRect.Width();

		windowRect.left += ((width - WINDOWSIZE_COLLAPSE) * WINDOWSIZE_RATE) + 1;

		if (width <= WINDOWSIZE_COLLAPSE)
		{
			StopTimer(ID_TMR_WINDOWCLOSE);
			return;
			windowRect.left = windowRect.right - WINDOWSIZE_COLLAPSE;
		}
		MoveWindow(windowRect, TRUE);
	}


	//CDialogEx::OnTimer(nIDEvent);
}


void CIPConfigureDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	DoLButtonClick();
	//CDialogEx::OnLButtonUp(nFlags, point);
}


void CIPConfigureDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	DoRButtonClick();
	//CDialogEx::OnRButtonUp(nFlags, point);
}


void CIPConfigureDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	DoLButtonDoubleClick();
	//CDialogEx::OnLButtonDblClk(nFlags, point);
}

void CIPConfigureDlg::DoLButtonClick()
{
	POINT pt;
	GetCursorPos(&pt);

	CMenu menu;
	menu.LoadMenu(IDM_TRAY_L);

	CMenu* pPopup = menu.GetSubMenu(0);

	// SetForgroundWindowとPostMessageが必要な理由は、
	// Knowledge Base (Q135788)参照のこと
	SetForegroundWindow();
	pPopup->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	PostMessage(WM_NULL);
}

void CIPConfigureDlg::DoRButtonClick()
{
	POINT pt;
	GetCursorPos(&pt);

	CMenu menu;
	menu.LoadMenu(IDM_TRAY_R);

	CMenu* pPopup = menu.GetSubMenu(0);

	// SetForgroundWindowとPostMessageが必要な理由は、
	// Knowledge Base (Q135788)参照のこと
	SetForegroundWindow();
	pPopup->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	PostMessage(WM_NULL);
}

void CIPConfigureDlg::DoLButtonDoubleClick()
{
	SendMessage(WM_COMMAND, ID_APP_ABOUT);
}


void CIPConfigureDlg::OnMouseHover(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	CDialogEx::OnMouseHover(nFlags, point);
}


void CIPConfigureDlg::OnMouseLeave()
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	StartTimer(ID_TMR_WINDOWCLOSE_DELAY);

	CDialogEx::OnMouseLeave();
}


void CIPConfigureDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect windowRect;
	GetWindowRect(windowRect);

	CRect rect;
	GetClientRect(rect);
	if (PtInRect(rect, point)) {

		// マウス監視開始(LEAVEイベント)
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = this->m_hWnd;
		_TrackMouseEvent(&tme);
	}

	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	StopTimer(ID_TMR_WINDOWCLOSE);
	StartTimer(ID_TMR_WINDOWOPEN);
}

/* 既にタイマーが作動しているときにSetTimerすると周期が乱れるのでちゃんとチェックする。 */
void CIPConfigureDlg::StartTimer(UINT_PTR nTimerID)
{
	switch (nTimerID)
	{
	case ID_TMR_WINDOWOPEN:
		if (m_WindowOpenTimerID == 0)
		{
			m_WindowOpenTimerID = SetTimer(ID_TMR_WINDOWOPEN, WINDOWOPENTIMER_INTERVAL, NULL);
		}
		break;

	case ID_TMR_WINDOWCLOSE:
		if (m_WindowCloseTimerID == 0)
		{
			m_WindowCloseTimerID = SetTimer(ID_TMR_WINDOWCLOSE, WINDOWCLOSETIMER_INTERVAL, NULL);
		}
		break;

	case ID_TMR_WINDOWCLOSE_DELAY:
		if (m_WindowCloseDelayTimerID == 0)
		{
			m_WindowCloseDelayTimerID = SetTimer(ID_TMR_WINDOWCLOSE_DELAY, WINDOWCLOSEDELAYTIMER_INTERVAL, NULL);
		}
		break;

	default:
		break;
	}
}

void CIPConfigureDlg::StopTimer(UINT_PTR nTimerID)
{
	switch (nTimerID)
	{
	case ID_TMR_WINDOWOPEN:
		if (m_WindowOpenTimerID != 0)
		{
			m_WindowOpenTimerID = 0;
			KillTimer(ID_TMR_WINDOWOPEN);
		}
		break;

	case ID_TMR_WINDOWCLOSE:
		if (m_WindowCloseTimerID != 0)
		{
			m_WindowCloseTimerID = 0;
			KillTimer(ID_TMR_WINDOWCLOSE);
		}
		break;

	case ID_TMR_WINDOWCLOSE_DELAY:
		if (m_WindowCloseDelayTimerID != 0)
		{
			m_WindowCloseDelayTimerID = 0;
			KillTimer(ID_TMR_WINDOWCLOSE_DELAY);
		}
		break;

	default:
		break;
	}
}

