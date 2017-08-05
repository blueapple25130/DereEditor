# include <Siv3D.hpp>
# include "Editor.h"

void Main()
{
	//TODO:可変ウィンドウサイズ対応
	//Window::SetStyle(WindowStyle::Sizeable);
	Window::Resize(1024, 768);

	//アセットロード
	TextureAsset::Register(L"notes", L"/2000");
	TextureAsset::Register(L"icons", L"/2001");
	SoundAsset::Register(L"tap", L"/3000");
	SoundAsset::Register(L"flick", L"/3001");
	SoundAsset::Register(L"slide", L"/3002");
	SoundAsset::Register(L"none", L"/3003");
	FontAsset::Register(L"editor", 16);

	Editor editor;

	Graphics::SetVSyncEnabled(true);

	while (System::Update()) {
		ClearPrint();
		Println(L"[Info]");
		Println(L"FPS:", Profiler::FPS());
		editor.update();
		editor.draw();
	}
}
