
// ChatClientDlg.h: 헤더 파일
//

#pragma once

class ServerSession;
using ServerSessionRef = shared_ptr<class ServerSession>;

// CChatClientDlg 대화 상자
class CChatClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CChatClientDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

public:
	void	AddEventString(const wchar_t* ap_string);
// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListBox m_chat_list;
	afx_msg void OnBnClickedSendBtn();

public:
	bool				_isConnected = false;
	ServerSessionRef	_serverSession;
public:
	afx_msg void OnBnClickedConnect();
	void	updateServerStatus();
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnLbnSelchangeChatList();
	CListBox m_room_list;
	CEdit m_name_edit;
};
