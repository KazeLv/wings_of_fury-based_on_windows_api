#include<windows.h>
#include<mmsystem.h>
#include<time.h>
#include<strsafe.h>

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Msimg32.lib")


#define WINDOW_TITLE L"飞机大战"
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 852

HDC hdc;
HDC g_mdc;
HDC g_bufdc;

int iScore = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void GameStart(HWND hwnd);
void GameUpdate(HWND hwnd);
void GameEnd(HWND hwnd);

enum GameState {
	GS_Menu,
	GS_Playing,
	GS_Result
};

GameState gameState;	

void ChangeGameStateTo(GameState gs, HWND hwnd);

struct GameMenu {
	HBITMAP hBG;
	HBITMAP hTitle;
	HBITMAP hLoading;

	void Init(HWND hwnd) {//当程序启动时进行初始化工作,相当于Awake
		hBG = (HBITMAP)LoadImage(NULL, L"image/background.bmp", IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);
		hTitle = (HBITMAP)LoadImage(NULL, L"image/title.bmp", IMAGE_BITMAP, 429, 84, LR_LOADFROMFILE);
		hLoading = (HBITMAP)LoadImage(NULL, L"image/game_loading.bmp", IMAGE_BITMAP, 176, 36, LR_LOADFROMFILE);
	}
	void Start(HWND hwnd) {//当进入到当前状态时会执行的初始化（每次回到菜单）
		SelectObject(g_bufdc, hBG);
		BitBlt(g_mdc, 0, 0, 480, 852, g_bufdc, 0, 0, SRCCOPY);

		SelectObject(g_bufdc, hTitle);
		TransparentBlt(g_mdc, 20, 50, 429, 84, g_bufdc, 0, 0, 429, 84, RGB(0, 0, 0));

		SelectObject(g_bufdc, hLoading); 
		TransparentBlt(g_mdc, 152, 600, 176, 36, g_bufdc, 0, 0, 176, 36, RGB(255, 255, 255));

		BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);

	}
	void Update(HWND hwnd) {//在当前状态下不断调用的Update函数

	}

	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_LBUTTONUP:
			ChangeGameStateTo(GS_Playing, hwnd);
			break;
		}
	}

	void Destroy(HWND hwnd) {//在当前状态下程序结束的清理工作

	}
};

struct Bullet {
	int x;
	int y;
	bool isExist = false;
};

struct Enemy {
	int x;
	int y;
	bool isDead = false;
	bool isExist = false;
	int iAnimationTimer = 0;
	int iAnimationIndex = 0;
};
bool IsInclude(Enemy, int, int);

struct GamePlaying {
	HBITMAP hBG;
	HBITMAP hHeroArray[2];
	HBITMAP hEnemyArray[5];
	HBITMAP hBullet;

	Bullet bulletsArray[30];
	Enemy enemyArray[30];

	int iPlayerPositionX = 190;
	int iPlayerPositionY = 600;

	int iBackgroundOffset = 0;
	int iBulletTimer = 0;
	int iHeroTimer = 0;
	int iHeroIndex = 0;
	int iEnemySpawnTimer = 0;

	bool isMouseDown = false;
	POINT preMousePoint;

	void Init(HWND hwnd) {//当程序启动时进行初始化工作,相当于Awake
		hBG = (HBITMAP)LoadImage(NULL, L"image/background.bmp", IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);
		hHeroArray[0] = (HBITMAP)LoadImage(NULL, L"image/hero1.bmp", IMAGE_BITMAP, 100, 124, LR_LOADFROMFILE);
		hHeroArray[1] = (HBITMAP)LoadImage(NULL, L"image/hero2.bmp", IMAGE_BITMAP, 100, 124, LR_LOADFROMFILE);
		hBullet = (HBITMAP)LoadImage(NULL, L"image/bullet1.bmp", IMAGE_BITMAP, 9, 21, LR_LOADFROMFILE);
		hEnemyArray[0] = (HBITMAP)LoadImage(NULL, L"image/enemy0.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);
		hEnemyArray[1] = (HBITMAP)LoadImage(NULL, L"image/enemy1.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);
		hEnemyArray[2] = (HBITMAP)LoadImage(NULL, L"image/enemy2.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);
		hEnemyArray[3] = (HBITMAP)LoadImage(NULL, L"image/enemy3.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);
		hEnemyArray[4] = (HBITMAP)LoadImage(NULL, L"image/enemy4.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);

		srand((unsigned)time(NULL));
	}
	void Start(HWND hwnd) {//当进入到当前状态时会执行的初始化（每次回到菜单）
		mciSendString(L"open sound/game_music.wav", NULL, 0, 0);
		mciSendString(L"play sound/game_music.wav", NULL, 0, 0);
	}
	void Update(HWND hwnd) {//在当前状态下不断调用的Update函数
		//渲染背景
		SelectObject(g_bufdc, hBG);
		iBackgroundOffset += 4;
		BitBlt(g_mdc, 0, iBackgroundOffset, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);
		BitBlt(g_mdc, 0, iBackgroundOffset - WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);
		if (iBackgroundOffset > WINDOW_HEIGHT) {
			iBackgroundOffset -= WINDOW_HEIGHT;
		}
		//渲染主角
		iHeroTimer++;
		if (iHeroTimer % 10 == 0) {
			iHeroIndex++;
			iHeroIndex %= 2;
		}
		SelectObject(g_bufdc, hHeroArray[iHeroIndex]);
		TransparentBlt(g_mdc, iPlayerPositionX, iPlayerPositionY, 100, 124, g_bufdc, 0, 0, 100, 124, RGB(255, 255, 255));
		//渲染子弹
		//生成子弹
		iBulletTimer++;
		if (iBulletTimer % 10 == 0) {
			for (int i = 0; i < 30; i++) {
				if (bulletsArray[i].isExist == false) {
					bulletsArray[i].x = iPlayerPositionX + 50-4;
					bulletsArray[i].y = iPlayerPositionY - 21;
					bulletsArray[i].isExist = true;
					break;
				}
			}

		}
		//子弹的绘制 飞行 消除
		SelectObject(g_bufdc, hBullet);
		for (int i = 0; i < 30; i++) {
			if (bulletsArray[i].isExist) {
				bulletsArray[i].y -= 10;
				if (bulletsArray[i].y < -21) {
					bulletsArray[i].isExist = false;
					continue;
				}
				TransparentBlt(g_mdc, bulletsArray[i].x, bulletsArray[i].y, 9, 21, g_bufdc, 0, 0, 9, 21, RGB(255, 255, 255));
			}
		}
		//主角的位置控制
		if (isMouseDown) {
			POINT nowMousePoint;
			GetCursorPos(&nowMousePoint);
			int xOffset = nowMousePoint.x - preMousePoint.x;
			int yOffset = nowMousePoint.y - preMousePoint.y;
			iPlayerPositionX += xOffset;
			iPlayerPositionY += yOffset;
			if (iPlayerPositionX <= 0) iPlayerPositionX = 0;
			if (iPlayerPositionX >= WINDOW_WIDTH - 100) iPlayerPositionX = WINDOW_WIDTH - 100;
			if (iPlayerPositionY <= 0) iPlayerPositionY = 0;
			if (iPlayerPositionY >= WINDOW_HEIGHT - 124) iPlayerPositionY = WINDOW_HEIGHT - 124;
			preMousePoint = nowMousePoint;
		}
		//敌人的生成
		iEnemySpawnTimer++;
		if (iEnemySpawnTimer % 50 == 0) {
			for (int i = 0; i < 30; i++) {
				if (enemyArray[i].isExist == false) {
					int x = rand() % (WINDOW_WIDTH - 51);
					int y = -39;
					enemyArray[i].x = x;
					enemyArray[i].y = y;
					enemyArray[i].isExist = true;
					enemyArray[i].isDead = false;
					enemyArray[i].iAnimationIndex = 0;
					enemyArray[i].iAnimationTimer = 0;
					break;
				}
			}
		}
		//敌人的绘制 移动
		for (int i = 0; i < 30; i++) {
			if (enemyArray[i].isExist) {
				if (enemyArray[i].isDead == false) {
					enemyArray[i].y += 5;
					SelectObject(g_bufdc, hEnemyArray[0]);
					TransparentBlt(g_mdc, enemyArray[i].x, enemyArray[i].y, 51, 39, g_bufdc, 0, 0, 51, 39, RGB(255, 255, 255));
					if (enemyArray[i].y > WINDOW_HEIGHT) {
						enemyArray[i].isExist = false;
						iScore--;
					}
				}
				else {
					enemyArray[i].iAnimationTimer++;
					if (enemyArray[i].iAnimationTimer % 10 == 0) {
						enemyArray[i].iAnimationIndex++;
					}
					if (enemyArray[i].iAnimationIndex == 5) {
						enemyArray[i].isExist = false;
					}
					SelectObject(g_bufdc, hEnemyArray[enemyArray[i].iAnimationIndex]);
					TransparentBlt(g_mdc, enemyArray[i].x, enemyArray[i].y, 51, 39, g_bufdc, 0, 0, 51, 39, RGB(255, 255, 255));
				}
			}
		}
		//敌人和子弹的碰撞检测
		for (int i = 0; i < 30; i++) {
			if (enemyArray[i].isDead==false) {
				for (int j = 0; j < 30; j++) {
					if (bulletsArray[j].isExist) {
						if (IsInclude(enemyArray[i], bulletsArray[j].x + 4, bulletsArray[j].y + 10)) {
							enemyArray[i].isDead = true;
							bulletsArray[j].isExist = false;
							iScore++;
							PlaySound(L"sound/enemy0_down.wav", nullptr, SND_ASYNC|SND_FILENAME);
							break;
						}
					}
				}
			}
		}
		//主角与敌人的碰撞
		for (int i = 0; i < 30; i++) {
			if (enemyArray[i].isExist&&enemyArray[i].isDead == false) {
				bool isInclude1 = IsInclude(enemyArray[i], iPlayerPositionX + 50, iPlayerPositionY);
				bool isInclude2 = IsInclude(enemyArray[i], iPlayerPositionX, iPlayerPositionY + 80);
				bool isInclude3 = IsInclude(enemyArray[i], iPlayerPositionX + 100, iPlayerPositionY + 80);
				if (isInclude1 || isInclude2 || isInclude3) {
					mciSendString(L"stop sound/game_music.wav", NULL, 0, 0);
					PlaySound(L"sound/game_over.wav", nullptr, SND_FILENAME | SND_ASYNC);
					ChangeGameStateTo(GS_Result, hwnd);
					break;
				}
			}
		}

		BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
	}

	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_LBUTTONUP:
			isMouseDown = false;
			break;
		case WM_LBUTTONDOWN:
			isMouseDown = true;
			GetCursorPos(&preMousePoint);
			break;
		}
	}

	void Destroy(HWND hwnd) {//在当前状态下程序结束的清理工作

	}
};

struct GameResult {

	HBITMAP hGameOver;
	TCHAR scoreText[10];
	size_t chLength = 0;

	void Init(HWND hwnd) {//当程序启动时进行初始化工作,相当于Awake
		hGameOver = (HBITMAP)LoadImage(NULL, L"image/gameover.bmp", IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);

		HFONT hFont = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("宋体"));
		SelectObject(g_mdc, hFont);//给mdc设置一个画笔
		SetBkMode(g_mdc, TRANSPARENT);
	}
	void Start(HWND hwnd) {//当进入到当前状态时会执行的初始化（每次回到菜单）
		swprintf_s(scoreText, L"%d", iScore);
	}
	void Update(HWND hwnd) {//在当前状态下不断调用的Update函数
		SelectObject(g_bufdc, hGameOver);
		BitBlt(g_mdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);//绘制背景

		TextOut(g_mdc, 225, 600, scoreText,wcslen(scoreText));

		BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
	}

	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	}

	void Destroy(HWND hwnd) {//在当前状态下程序结束的清理工作

	}
};

GameMenu gameMenu;
GamePlaying gamePlaying;
GameResult gameResult;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	MSG msg = {};

	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = L"MyClass";
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wndClass)) {
		return -1;
	}

	HWND hwnd = CreateWindow(L"MyClass", WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
	
	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	//游戏初始化
	GameStart(hwnd);

	DWORD tNow = GetTickCount();
	DWORD tPre = GetTickCount();

	//消息处理
	while (msg.message!=WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			tNow = GetTickCount();
			if (tNow - tPre > 10) {
				GameUpdate(hwnd);
				tPre = GetTickCount();//通过计时来控制GameUpdate函数的调用（类似于Unity的Update）
			}
		}
	}


	GameEnd(hwnd);
	UnregisterClass(L"MyClass", hInstance);

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_KEYDOWN:
		switch (gameState) {
		case GS_Menu:
			gameMenu.OnWindowMessage(hwnd, message, wParam, lParam);
			break;
		case GS_Playing:
			gamePlaying.OnWindowMessage(hwnd, message, wParam, lParam);
			break;
		case GS_Result:
			gameResult.OnWindowMessage(hwnd, message, wParam, lParam);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void GameStart(HWND hwnd) {
	gameMenu.Init(hwnd);
	gamePlaying.Init(hwnd);
	gameResult.Init(hwnd);

	hdc = GetDC(hwnd);
	g_mdc = CreateCompatibleDC(hdc);
	g_bufdc = CreateCompatibleDC(hdc);

	HBITMAP emptyBmp = CreateCompatibleBitmap(hdc, 480, 852);
	SelectObject(g_mdc, emptyBmp);

	gameState = GS_Menu;//设置默认状态为Menu
	gameMenu.Start(hwnd);
}

void GameUpdate(HWND hwnd) {
	switch (gameState) {
	case GS_Menu:
		gameMenu.Update(hwnd);
		break;
	case GS_Playing:
		gamePlaying.Update(hwnd);
			break;
	case GS_Result:
		gameResult.Update(hwnd);
		break;
	}
}

void GameEnd(HWND hwnd) {
	gameMenu.Destroy(hwnd);
	gamePlaying.Destroy(hwnd);
	gameResult.Destroy(hwnd);
	DeleteDC(g_bufdc);
	DeleteDC(g_mdc);
	ReleaseDC(hwnd,hdc);
}

void ChangeGameStateTo(GameState gs, HWND hwnd) {
	gameState = gs;
	switch (gs) {
	case GS_Menu:
		gameMenu.Start(hwnd);
		break;
	case GS_Playing:
		gamePlaying.Start(hwnd);
		break;
	case GS_Result:
		gameResult.Start(hwnd);
		break;
	}
}

bool IsInclude(Enemy enemy, int x, int y) {
	if (x > enemy.x&&x<(enemy.x + 51) && y>enemy.y&&y < (enemy.y + 39)) {
		return true;
	}
	return false;
}