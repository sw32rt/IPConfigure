
// IPConfigure.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CIPConfigureApp:
// このクラスの実装については、IPConfigure.cpp を参照してください
//

class CIPConfigureApp : public CWinApp
{
public:
	CIPConfigureApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CIPConfigureApp theApp;
