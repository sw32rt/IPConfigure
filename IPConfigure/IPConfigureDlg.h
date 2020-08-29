
// IPConfigureDlg.h : ヘッダー ファイル
//

#pragma once
#include <vector>

#define WINDOWOPENTIMER_INTERVAL        1
#define WINDOWCLOSETIMER_INTERVAL       1
#define WINDOWCLOSEDELAYTIMER_INTERVAL  1

#define WINDOWSIZE_FIXHEIGHT            200UL

#define WINDOWSIZE_EXPAND               300UL
#define WINDOWSIZE_COLLAPSE             10UL                  
#define WINDOWSIZE_RATE                 0.2

// CIPConfigureDlg ダイアログ
class CIPConfigureDlg : public CDialogEx
{
private:
	HICON m_hIcon_main;
	HICON m_hIcon_current;
	std::vector<CString> m_DisplayStringVector;
	CBrush m_brDlg;		// ダイアログの背景色ブラシ(ここを追加する)
	BOOL m_bFireDoubleClick;
	int m_WindowOpenTimerID;
	int m_WindowCloseTimerID;
	int m_WindowCloseDelayTimerID;

// コンストラクション
public:
	CIPConfigureDlg(CWnd* pParent = nullptr);	// 標準コンストラクター

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IPCONFIGURE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	BOOL NotifyIcon(DWORD dwMessage, HICON hIcon, LPCTSTR pszTip = NULL);
	BOOL NotifyIcon(DWORD dwMessage, HICON hIcon, UINT nStringResource);
	LRESULT OnTrayNotify(UINT wParam, LONG lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	void UpdateIPAddress();
	static UINT IPAddressChangeWatchThread(LPVOID p);
	void OnIPAddressChanged();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	void DoLButtonClick();
	void DoRButtonClick();
	void DoLButtonDoubleClick();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	void StartTimer(UINT_PTR nTimerID);
	void StopTimer(UINT_PTR nTimerID);
};
