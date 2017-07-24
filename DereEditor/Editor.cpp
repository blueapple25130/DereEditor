#include "Editor.h"

void Editor::updateNoteState() {
	m_notes.clear();
	double plevNoteTick = 0;
	double plevNoteTime = 0;//Offset
	double currentTempo = 160;//StandardTempo
	double currentMeasureRealTick = 0;
	int measureIndex = 0;
	for (auto& measure : m_measures) {
		for (auto& note : measure->getNotes()) {
			double currentNoteTick = currentMeasureRealTick + note->Tick;
			note->Time = plevNoteTime + (currentNoteTick - plevNoteTick) / 192 * 60 / currentTempo;
			plevNoteTick = currentNoteTick;
			plevNoteTime = note->Time;
			if (typeid(*note) == typeid(PlayableNote))
			{
				note->setPosition(getLaneOffset() + dynamic_cast<PlayableNote*>(note.get())->Lane * m_LaneWidth, static_cast<int>(-currentNoteTick / 192 * getBeatPerHeight()));
			}
			else if (typeid(*note) == typeid(ChangeTempo)) {
				note->setPosition(512, static_cast<int>(-currentNoteTick / 192 * getBeatPerHeight()));
				currentTempo = dynamic_cast<ChangeTempo*>(note.get())->getTempo();
				//TempoChangeLeap.Add(note);
			}
		}
		m_notes.insert(m_notes.end(), measure->getNotes().begin(), measure->getNotes().end());
		++measureIndex;

		currentMeasureRealTick += measure->getRealTick();
	}

	// TODO:ノート間の接続を実装
	// TODO:テンポを考慮したスクロールを実装

}

double Editor::getBeatPerHeight() {
	return m_zoom * BeatPerHeight;
}

//TODO:可変ウィンドウサイズ対応

int Editor::getLaneOffset() {
	return m_LaneOffset;
}

void Editor::setOrRemoveNote(std::vector<std::shared_ptr<Note>> &notes, int tick, int lane) {
	for (auto it = notes.begin(); it != notes.end();) {
		if ((*it)->Tick == tick && (*it)->Lane == lane) {
			notes.erase(it);
			return;
		}
		else {
			++it;
		}
	}
	if (lane == 5) {
		notes.push_back(std::make_shared<ChangeTempo>(tick, Parse<double>(m_gui.textField(L"tempo").text)));
	}
	else {
		notes.push_back(std::make_shared<PlayableNote>(tick, lane, NoteType::Tap));
	}
}

Editor::Editor() :
	m_rect(0, 0, 512, Window::ClientRect().h),
	m_gui(GUIStyle::Default)
{
	//GUI初期化

	m_gui.addln(GUIText::Create(L"グリッド"));
	m_gui.add(GUIText::Create(L"ズーム"));
	m_gui.add(L"zoom", GUISlider::Create(0.2, 2.0, 1.0));
	m_gui.addln(L"zoom-val", GUIText::Create(L"100%"));

	m_gui.add(L"hr", GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"テンポ変更"));
	m_gui.add(GUIText::Create(L"BPM"));
	m_gui.addln(L"tempo", GUITextField::Create(none));

	m_gui.add(L"hr", GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"小節"));
	m_gui.add(L"add-measure", GUIButton::Create(L"追加"));
	m_gui.addln(L"remove-measure", GUIButton::Create(L"削除"));
	m_gui.add(L"tf1", GUITextField::Create(none));
	m_gui.add(L"set-measure", GUIButton::Create(L"変更"));
	m_gui.setPos(m_rect.tr);

	for (int i = 0; i < 200; ++i) {
		m_measures.push_back(std::make_shared<Measure>());
	}
}

void Editor::update() {
	//GUI関連
	if (m_gui.slider(L"zoom").hasChanged) {
		m_zoom = m_gui.slider(L"zoom").value;
		m_gui.text(L"zoom-val").text = Format(static_cast<int>(m_zoom * 100), L"%");
		updateNoteState();
	}

	//TODO:キーショートカット

	if (m_rect.leftClicked) {

		Point mousePos = Mouse::Pos().movedBy(0, -Window::ClientRect().h);

		int lane = 0;
		int offset = getLaneOffset() - m_LaneWidth / 2;
		for (int i = 0; i < 6; i++)
		{
			if (offset + i * m_LaneWidth <= mousePos.x && mousePos.x < offset + (i + 1) * m_LaneWidth)
			{
				lane = i;
			}
		}

		double gridHeightHalf = getBeatPerHeight() * 4 / m_division / 2;

		for (auto it = m_measures.begin(); it != m_measures.end(); ++it) {

			int lineCount = static_cast<int>(Ceil(m_division * (*it)->getLength()));
			double y = (*it)->BeginY;

			//拍子を考慮して判定する
			//(分割線が小節開始線のみ?小節の中間:小節開始線-グリッドの高さ/2)以上、小節開始線と前の小節の最後の分割線の中間未満
			if (y - (lineCount == 1 ? (y - (*it)->EndY) / 2 : gridHeightHalf) <= mousePos.y && mousePos.y < y + (it == m_measures.begin() ? gridHeightHalf : ((*(it - 1))->LastLineY - y) / 2))
			{
				setOrRemoveNote((*it)->getNotes(), 0, lane);
			}
			y -= gridHeightHalf * 2;

			//中間
			//分割線が小節開始線のみの場合、スキップ
			//分割線-グリッドの高さ/2の以上、分割線+グリッドの高さ/2未満で判定
			for (int i = 1; i < lineCount - 1; i++)
			{
				if (y - gridHeightHalf <= mousePos.y && mousePos.y < y + gridHeightHalf)
				{
					int tick = 192 / (m_division / 4) * i;
					setOrRemoveNote((*it)->getNotes(), tick, lane);
				}
				y -= gridHeightHalf * 2;
			}
			//最後
			//分割線が小節開始線のみの場合、スキップ
			//分割線と小節終線(次の小節の開始線)の中間以上、分割線+グリッドの高さ/2未満で判定
			if (lineCount != 1 && y - (y - (*it)->EndY) / 2 <= mousePos.y && mousePos.y < y + gridHeightHalf)
			{
				int tick = 192 / (m_division / 4) * (lineCount - 1);
				setOrRemoveNote((*it)->getNotes(), tick, lane);
			}
		}
		updateNoteState();
	}
}

void Editor::draw() {

	Graphics2D::ClearTransform();

	//m_rect.draw(Color(30, 30, 30));

	//レーン
	for (auto i = 0; i < 5; i++) {
		Rect(i * m_LaneWidth + getLaneOffset(), Window::ClientRect().y, 1, Window::ClientRect().h).draw(Palette::White);
	}

	Rect(5 * m_LaneWidth + getLaneOffset() + 20, Window::ClientRect().y, 1, Window::ClientRect().h).draw(Color(50, 50, 50));

	Graphics2D::SetTransform(Mat3x2::Identity().translate(0, Window::ClientRect().h));

	//小節
	double measureBeginY = 0;
	for (auto& measure : m_measures) {
		int lineCount = static_cast<int>(Ceil(m_division * measure->getLength()));

		double gridHeight = getBeatPerHeight() * 4 / m_division;
		double y = measureBeginY;

		Rect(getLaneOffset() - m_LaneWidth / 2, static_cast<int>(y), 462, 1).draw(Palette::Green);

		int emphasizeInterval = m_division / 4;

		for (int i = 1; i < lineCount; i++)
		{
			y -= gridHeight;
			Rect(getLaneOffset() - m_LaneWidth / 2, static_cast<int>(y), 462, 1).draw(i % emphasizeInterval == 0 ? Palette::White : Palette::Gray);
		}

		measure->BeginY = measureBeginY;
		measure->LastLineY = y;
		measureBeginY -= getBeatPerHeight() * 4 * measure->getLength();
		measure->EndY = measureBeginY;
	}

	//小節番号
	for (size_t i = 0; i < m_measures.size(); ++i) {
		FontAsset(L"editor")(Pad(i, { 3, L'0' })).drawAt(getLaneOffset() - 70, m_measures[i]->BeginY);
	}

	//ノート
	for (auto& note : m_notes) {
		if (typeid(*note) == typeid(PlayableNote))
			note->draw();
	}

	//テンポ変更
	for (auto& note : m_notes) {
		if (typeid(*note) == typeid(ChangeTempo))
			note->draw();
	}

	//小節選択
	Triangle(getLaneOffset() - 110, m_measures[m_selectedMeasure]->BeginY, 15, 90_deg).draw(Palette::Red);

	Println(L"Notes:", m_notes.size());
}
