#include "Editor.h"

void Editor::updateNoteState() {
	m_notes.clear();
	m_playableNotes.clear();
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
				note->setPosition(getLaneOffset() + std::dynamic_pointer_cast<PlayableNote>(note)->Lane * m_LaneWidth, static_cast<int>(-currentNoteTick / 192 * getBeatPerHeight()));
				m_playableNotes.push_back(std::dynamic_pointer_cast<PlayableNote>(note));
			}
			else if (typeid(*note) == typeid(ChangeTempo)) {
				note->setPosition(512, static_cast<int>(-currentNoteTick / 192 * getBeatPerHeight()));
				currentTempo = std::dynamic_pointer_cast<ChangeTempo>(note)->getTempo();
				//TempoChangeLeap.Add(note);
			}
		}
		m_notes.insert(m_notes.end(), measure->getNotes().begin(), measure->getNotes().end());
		++measureIndex;

		currentMeasureRealTick += measure->getRealTick();
	}

	std::sort(m_notes.begin(), m_notes.end(), [](const std::shared_ptr<Note>& a, const std::shared_ptr<Note>& b)
	{
		return a->Time < b->Time;
	});


	std::sort(m_playableNotes.begin(), m_playableNotes.end(), [](const std::shared_ptr<PlayableNote>& a, const std::shared_ptr<PlayableNote>& b)
	{
		return a->Time < b->Time;
	});

	// TODO:テンポを考慮したスクロールを実装

	for (auto currentNote = m_playableNotes.begin(); currentNote != m_playableNotes.end(); ++currentNote) {
		(*currentNote)->ConnectNote = nullptr;
		switch ((*currentNote)->getNoteType())
		{
		case NoteType::LFlick:
		case NoteType::RFlick:
		{
			auto connectNote = std::find_if(m_playableNotes.begin(), m_playableNotes.end(), [&](std::shared_ptr<PlayableNote> x) {return x->Time > (*currentNote)->Time &&  x->Channel == (*currentNote)->Channel && x->Time - (*currentNote)->Time < 1.0; });
			if (connectNote != m_playableNotes.end()) {
				if ((*connectNote)->Channel % 4 < 2) {
					if ((*connectNote)->Lane < (*currentNote)->Lane && (*currentNote)->getNoteType() == NoteType::LFlick && ((*connectNote)->getNoteType() == NoteType::LFlick || (*connectNote)->getNoteType() == NoteType::RFlick))
					{
						(*currentNote)->ConnectNote = (*connectNote);
					}
					else if ((*connectNote)->Lane > (*currentNote)->Lane && (*currentNote)->getNoteType() == NoteType::RFlick && ((*connectNote)->getNoteType() == NoteType::LFlick || (*connectNote)->getNoteType() == NoteType::RFlick))
					{
						(*currentNote)->ConnectNote = (*connectNote);

					}
				}
				else if((*connectNote)->getNoteType() == NoteType::LFlick || (*connectNote)->getNoteType() == NoteType::RFlick){
					(*currentNote)->ConnectNote = (*connectNote);
				}
			}
		}
		break;
		case NoteType::Long:
		{
			auto connectNote = std::find_if(m_playableNotes.begin(), m_playableNotes.end(), [&](std::shared_ptr<PlayableNote> x) {return x->Time > (*currentNote)->Time && x->Lane == (*currentNote)->Lane; });
			if (connectNote != m_playableNotes.end())
				(*currentNote)->ConnectNote = (*connectNote);
		}
		break;
		case NoteType::Slide:
		{
			auto connectNote = std::find_if(m_playableNotes.begin(), m_playableNotes.end(), [&](std::shared_ptr<PlayableNote> x) {return x->Time > (*currentNote)->Time && x->Channel == (*currentNote)->Channel; });
			if (connectNote != m_playableNotes.end())
				(*currentNote)->ConnectNote = (*connectNote);
		}
		break;
		}
	}
}

double Editor::getBeatPerHeight() {
	return m_zoom * BeatPerHeight;
}

//TODO:可変ウィンドウサイズ対応

int Editor::getLaneOffset() {
	return m_LaneOffset;
}

void Editor::setOrRemoveNote(std::vector<std::shared_ptr<Note>> &notes, int tick, int lane, NoteType type) {
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
		notes.push_back(std::make_shared<PlayableNote>(tick, lane, m_channel, type));
	}
}

void Editor::changeZoomRatio() {
	m_zoom = m_gui.slider(L"zoom").value;
	m_gui.text(L"zoom-ratio").text = Format(static_cast<int>(m_zoom * 100), L"%");
	updateNoteState();
}

void Editor::addMeasure() {
	if (m_measures.size() == MEASURE_MAX) {
		MessageBox::Show(L"これ以上小節を追加出来ません。");
		return;
	}

	m_measures.insert(m_measures.begin() + m_selectedMeasure, std::make_shared<Measure>());
	++m_selectedMeasure;
	updateNoteState();
}

void Editor::removeMeasure() {
	if (m_measures.size() == MEASURE_MIN) {
		MessageBox::Show(L"小節を全て削除することは出来ません。");
		return;
	}

	if (m_measures[m_selectedMeasure]->getNotes().size() > 0) {
		if (MessageBox::Show(L"ノートが含まれています。小節を削除しますか？", MessageBoxStyle::YesNo) == MessageBoxCommand::Yes) {
			m_measures.erase(m_measures.begin() + m_selectedMeasure--);
			if (m_measures.size() == m_selectedMeasure) {
				--m_selectedMeasure;
			}
			updateNoteState();
		}
	}
	else {
		m_measures.erase(m_measures.begin() + m_selectedMeasure);
		if (m_measures.size() == m_selectedMeasure) {
			--m_selectedMeasure;
		}
		updateNoteState();
	}
}

void Editor::setMeasureLength() {
	double length = Parse<double>(m_gui.textField(L"measure-length").text);
	if (length <= 0) {
		MessageBox::Show(L"小節の長さは0以下に設定出来ません。");
		return;
	}

	//TODO:縮小範囲にノートが含まれている場合の処理を実装
	m_measures[m_selectedMeasure]->setLength(length);
	updateNoteState();
}

void Editor::channelUp() {
	m_gui.text(L"channel").text = Format(++m_channel);
}

void Editor::channelDown() {
	if (m_channel != 0)
		m_gui.text(L"channel").text = Format(--m_channel);
}

Editor::Editor() :
	m_rect(0, 0, 512, Window::ClientRect().h),
	m_gui(GUIStyle::Default)
{
	//GUI初期化

	m_gui.setTitle(L"メニュー");
	m_gui.addln(GUIText::Create(L"[グリッド]"));
	m_gui.add(GUIText::Create(L"ズーム"));
	m_gui.add(L"zoom", GUISlider::Create(0.2, 2.0, 1.0));
	m_gui.addln(L"zoom-ratio", GUIText::Create(L"100%"));

	m_gui.add(GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"[ノート]"));
	m_gui.add(GUIText::Create(L"チャンネル　"));
	m_gui.add(L"channel-down", GUIButton::Create(L"-"));
	m_gui.add(L"channel", GUIText::Create(L"0"));
	m_gui.addln(L"channel-up", GUIButton::Create(L"+"));

	m_gui.add(GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"[拡張ノート]"));
	m_gui.add(GUIText::Create(L"テンポ変更　"));
	m_gui.addln(L"tempo", GUITextField::Create(none));

	m_gui.add(GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"[小節]"));
	m_gui.add(L"add-measure", GUIButton::Create(L"追加"));
	m_gui.addln(L"remove-measure", GUIButton::Create(L"削除"));
	m_gui.add(L"measure-length", GUITextField::Create(none));
	m_gui.add(L"set-measure", GUIButton::Create(L"変更"));

	m_gui.add(GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"[音源]"));
	m_gui.add(GUIText::Create(L"基準テンポ　"));
	m_gui.addln(L"measure-length", GUITextField::Create(none));
	m_gui.add(GUIText::Create(L"オフセット　"));
	m_gui.addln(L"measure-length", GUITextField::Create(none));


	m_gui.setPos(m_rect.tr);

	for (size_t i = 0; i < MEASURE_DEFAULT; ++i) {
		m_measures.push_back(std::make_shared<Measure>());
	}
}

void Editor::update() {
	//GUI関連
	if (m_gui.slider(L"zoom").hasChanged) {
		changeZoomRatio();
	}
	else if (m_gui.button(L"add-measure").pushed) {
		addMeasure();
	}
	else if (m_gui.button(L"remove-measure").pushed) {
		removeMeasure();
	}
	else if (m_gui.button(L"set-measure").pushed) {
		setMeasureLength();
	}
	else if (m_gui.button(L"channel-up").pushed) {
		channelUp();
	}
	else if (m_gui.button(L"channel-down").pushed) {
		channelDown();
	}

	//TODO:キーショートカット
	if (Input::KeyControl.pressed) {
		if (Input::KeyS.clicked) {
			//TODO:Save
		}
		else if (Input::KeyZ.clicked) {
			//TODO:Undo
		}
		else if (Input::KeyY.clicked) {
			//TODO:Redo
		}
	}

	NoteType currentNoteType = NoteType::Tap;

	//ノート選択
	if (Input::KeyControl.pressed && Input::MouseL.clicked) {
		currentNoteType = NoteType::Slide;
	}
	else if (Input::KeyShift.pressed) {
		if (Input::MouseL.clicked) {
			currentNoteType = NoteType::LFlick;
		}
		else if (Input::MouseR.clicked) {
			currentNoteType = NoteType::RFlick;
		}
	}
	else {
		if (Input::MouseL.clicked) {
			currentNoteType = NoteType::Tap;
		}
		else if (Input::MouseR.clicked) {
			currentNoteType = NoteType::Long;
		}
	}


	if (m_rect.leftClicked || m_rect.rightClicked) {

		Point mousePos = Mouse::Pos().movedBy(0, -Window::ClientRect().h + m_offset);

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
				setOrRemoveNote((*it)->getNotes(), 0, lane, currentNoteType);
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
					setOrRemoveNote((*it)->getNotes(), tick, lane, currentNoteType);
				}
				y -= gridHeightHalf * 2;
			}
			//最後
			//分割線が小節開始線のみの場合、スキップ
			//分割線と小節終線(次の小節の開始線)の中間以上、分割線+グリッドの高さ/2未満で判定
			if (lineCount != 1 && y - (y - (*it)->EndY) / 2 <= mousePos.y && mousePos.y < y + gridHeightHalf)
			{
				int tick = 192 / (m_division / 4) * (lineCount - 1);
				setOrRemoveNote((*it)->getNotes(), tick, lane, currentNoteType);
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

	Graphics2D::SetTransform(Mat3x2::Identity().translate(0, Window::ClientRect().h - m_offset));

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

	//リボン
	for (auto& note : m_playableNotes) {
		note->drawRibbon();
	}

	//ノート
	for (auto& note : m_playableNotes) {
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
