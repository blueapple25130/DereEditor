#include "Editor.h"

void Editor::updateNoteState() {
	m_playableNotes.clear();
	m_changeTempos.clear();
	double plevNoteTick = 0;
	double plevNoteTime = m_offset / 1000;//Offset
	double currentTempo = m_standardTempo;//StandardTempo
	double currentMeasureRealTick = 0;
	int measureIndex = 0;
	for (auto& measure : m_measures) {
		std::sort(measure->getNotes().begin(), measure->getNotes().end(), [](const std::shared_ptr<Note>& a, const std::shared_ptr<Note>& b)
		{
			return a->Tick < b->Tick;
		});
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
				note->setPosition(getLaneOffset() + m_LaneWidth*m_laneCount, static_cast<int>(-currentNoteTick / 192 * getBeatPerHeight()));
				currentTempo = std::dynamic_pointer_cast<ChangeTempo>(note)->getTempo();
				m_changeTempos.push_back(std::dynamic_pointer_cast<ChangeTempo>(note));
			}
		}
		++measureIndex;

		currentMeasureRealTick += measure->getRealTick();
	}

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
				else if ((*connectNote)->getNoteType() == NoteType::LFlick || (*connectNote)->getNoteType() == NoteType::RFlick) {
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

void Editor::setOrRemovePlaybleNote(std::vector<std::shared_ptr<Note>> &notes, int tick, int lane, NoteType type) {
	for (auto it = notes.begin(); it != notes.end();) {
		if (typeid(**it) == typeid(PlayableNote) && (*it)->Tick == tick && std::dynamic_pointer_cast<PlayableNote>(*it)->Lane == lane) {
			notes.erase(it);
			return;
		}
		else {
			++it;
		}
	}
	notes.push_back(std::make_shared<PlayableNote>(tick, lane, m_channel, type));
}

void Editor::setOrRemoveChangeTempo(std::vector<std::shared_ptr<Note>> &notes, int tick, double tempo) {
	for (auto it = notes.begin(); it != notes.end();) {
		if (typeid(**it) == typeid(ChangeTempo) && (*it)->Tick == tick) {
			notes.erase(it);
			return;
		}
		else {
			++it;
		}
	}
	notes.push_back(std::make_shared<ChangeTempo>(tick, tempo));
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

	if (m_measures[m_selectedMeasure]->getNotes().end() != std::find_if(m_measures[m_selectedMeasure]->getNotes().begin(), m_measures[m_selectedMeasure]->getNotes().end(), [&](std::shared_ptr<Note> x) {return x->Tick >= length * 768; })) {
		if (MessageBox::Show(L"ノートが含まれています。小節の長さを変更しますか？", MessageBoxStyle::YesNo) == MessageBoxCommand::Yes) {
			for (auto it = m_measures[m_selectedMeasure]->getNotes().begin(); it != m_measures[m_selectedMeasure]->getNotes().end();) {
				if ((*it)->Tick >= length * 768) {
					it = m_measures[m_selectedMeasure]->getNotes().erase(it);
				}
				else {
					++it;
				}
			}
		}
		else {
			return;
		}
	}
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

double Editor::getScroll() {
	double currentTime = getMusic().streamPosSec();
	if (m_changeTempos.size() == 0) {
		return -(currentTime - m_offset / 1000) * m_standardTempo / 60.0  * getBeatPerHeight();
	}
	else {
		if (currentTime < (*m_changeTempos.begin())->Time)
		{
			return Lerp(m_offset / 1000 * m_standardTempo / 60.0  * getBeatPerHeight(), (*m_changeTempos.begin())->getY(), currentTime / (*m_changeTempos.begin())->Time);
		}
		else if ((*(m_changeTempos.end() - 1))->Time <= currentTime)
		{
			return (*(m_changeTempos.end() - 1))->getY() - (currentTime - (*(m_changeTempos.end() - 1))->Time) * (*(m_changeTempos.end() - 1))->getTempo() / 60 * getBeatPerHeight();
		}
		else
		{
			for (auto it = m_changeTempos.begin(); it != m_changeTempos.end() - 1; it++) {
				if ((*it)->Time <= currentTime && currentTime < (*(it + 1))->Time)
				{
					double t = (currentTime - (*it)->Time) / ((*(it + 1))->Time - (*it)->Time);
					return Lerp((*it)->getY(), (*(it + 1))->getY(), t);
				}
			}
			return 0;
		}
	}
}

void Editor::playSE() {
	double currentTime = getMusic().streamPosSec();
	for (auto &note : m_playableNotes) {
		if (!note->IsJudged && note->Time - currentTime < 0.0) {
			switch (note->getNoteType())
			{
			case NoteType::Tap:
			case NoteType::Long:
				SoundAsset(L"tap").playMulti();
				break;
			case NoteType::LFlick:
			case NoteType::RFlick:
				SoundAsset(L"flick").playMulti();
				break;
			case NoteType::Slide:
				SoundAsset(L"slide").playMulti();
				break;
			default:
				break;
			}
			note->IsJudged = true;
		}
	}
}

const SoundAsset& Editor::getMusic() {
	return SoundAsset(L"music");
}


void Editor::addLane() {
	//TODO:レーン削除
}

void Editor::removeLane() {
	//TODO:レーン追加
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
	m_gui.add(L"measure-count", GUITextField::Create(none));
	m_gui.add(GUIText::Create(L"個"));
	m_gui.addln(L"add-measure", GUIButton::Create(L"追加"));
	m_gui.addln(L"remove-measure", GUIButton::Create(L"削除"));
	m_gui.add(L"measure-length", GUITextField::Create(none));
	m_gui.add(L"set-measure", GUIButton::Create(L"変更"));

	m_gui.add(GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"[音源]"));
	m_gui.add(GUIText::Create(L"基準テンポ　"));
	m_gui.addln(L"standard-tempo", GUITextField::Create(none));
	m_gui.add(GUIText::Create(L"オフセット　"));
	m_gui.addln(L"offset", GUITextField::Create(none));

	m_gui.addln(GUIText::Create(L"[プレイヤー]"));
	m_gui.add(L"play-music", GUIButton::Create(L"Play"));
	m_gui.addln(L"pause-music", GUIButton::Create(L"Pause"));
	m_gui.addln(L"seek-music", GUISlider::Create(0.0, 1.0, 0.0, 300));

	m_gui.textField(L"tempo").setText(L"120");
	m_gui.textField(L"measure-length").setText(L"4/4");
	m_gui.textField(L"standard-tempo").setText(L"120");
	m_gui.textField(L"offset").setText(L"0");

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
	else if (m_gui.textField(L"standard-tempo").hasChanged) {
		m_standardTempo = Parse<double>(m_gui.textField(L"standard-tempo").text);
		updateNoteState();
	}
	else if (m_gui.textField(L"offset").hasChanged) {
		m_offset = Parse<double>(m_gui.textField(L"offset").text);
		updateNoteState();
	}
	else if (m_gui.button(L"play-music").pushed)
	{
		getMusic().play();
	}
	else if (m_gui.button(L"pause-music").pushed)
	{
		getMusic().pause();
	}
	else if (m_gui.slider(L"seek-music").hasChanged)
	{
		double seekTime = getMusic().lengthSec() * m_gui.slider(L"seek-music").value;
		getMusic().setPosSec(seekTime);
		for (auto &note : m_playableNotes)
			note->IsJudged = note->Time - seekTime < 0.0;
	}

	m_gui.button(L"play-music").enabled = (getMusic() && !getMusic().isPlaying());
	m_gui.button(L"pause-music").enabled = getMusic().isPlaying();
	m_gui.slider(L"seek-music").setValue(getMusic().streamPosSec() / getMusic().lengthSec());

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

	if (!getMusic().isPlaying()) {
		double seekTime = getMusic().streamPosSec() - 0.1*Mouse::Wheel();
		seekTime =
			seekTime < 0 ? seekTime = 0 :
			getMusic().lengthSec() < seekTime ? getMusic().lengthSec() :
			seekTime;
		getMusic().setPosSec(seekTime);
		for (auto &note : m_playableNotes)
			note->IsJudged = note->Time - seekTime < 0.0;
	}


	if (m_rect.leftClicked || m_rect.rightClicked) {

		Vec2 mousePos = Mouse::PosF().movedBy(0, -Window::ClientRect().h + m_gridOffset + getScroll());

		int lane = -1;
		int offset = getLaneOffset() - m_LaneWidth / 2;
		EditType editType = EditType::None;
		int allLaneCount = m_laneCount + 1;
		for (int i = 0; i < allLaneCount + 1; i++)
		{
			if (offset + i * m_LaneWidth <= mousePos.x && mousePos.x < offset + (i + 1) * m_LaneWidth)
			{
				lane = i;
			}
		}
		if (0 <= lane && lane < m_laneCount) {
			editType = EditType::PlayableNote;
		}
		else if (lane == m_laneCount) {
			editType = EditType::ChangeTempo;
		}
		else {
			editType = EditType::None;
		}

		if (editType != EditType::None) {
			double gridHeightHalf = getBeatPerHeight() * 4 / m_division / 2;

			for (auto it = m_measures.begin(); it != m_measures.end(); ++it) {

				int lineCount = static_cast<int>(Ceil(m_division * (*it)->getLength()));
				double y = (*it)->BeginY;
				int tick = -1;

				//拍子を考慮して判定する
				//(分割線が小節開始線のみ?小節の中間:小節開始線-グリッドの高さ/2)以上、小節開始線と前の小節の最後の分割線の中間未満
				if (y - (lineCount == 1 ? (y - (*it)->EndY) / 2 : gridHeightHalf) <= mousePos.y && mousePos.y < y + (it == m_measures.begin() ? gridHeightHalf : ((*(it - 1))->LastLineY - y) / 2))
					tick = 0;
				y -= gridHeightHalf * 2;

				//中間
				//分割線が小節開始線のみの場合、スキップ
				//分割線-グリッドの高さ/2の以上、分割線+グリッドの高さ/2未満で判定
				for (int i = 1; i < lineCount - 1; i++)
				{
					if (y - gridHeightHalf <= mousePos.y && mousePos.y < y + gridHeightHalf)
						tick = 192 / (m_division / 4) * i;
					y -= gridHeightHalf * 2;
				}
				//最後
				//分割線が小節開始線のみの場合、スキップ
				//分割線と小節終線(次の小節の開始線)の中間以上、分割線+グリッドの高さ/2未満で判定
				if (lineCount != 1 && y - (y - (*it)->EndY) / 2 <= mousePos.y && mousePos.y < y + gridHeightHalf)
					tick = 192 / (m_division / 4) * (lineCount - 1);

				if (tick != -1) {
					switch (editType)
					{
					case EditType::PlayableNote:
						setOrRemovePlaybleNote((*it)->getNotes(), tick, lane, currentNoteType);
						break;
					case EditType::ChangeTempo:
						setOrRemoveChangeTempo((*it)->getNotes(), tick, Parse<double>(m_gui.textField(L"tempo").text));
						break;
					}
				}
			}
			updateNoteState();
		}
	}
	playSE();
}

void Editor::draw() {

	Graphics2D::ClearTransform();

	const Mat3x2 scrollMatrix = Mat3x2::Identity().translate(0, Window::ClientRect().h - m_gridOffset - getScroll());

	const Line divLine(getLaneOffset() - m_LaneWidth / 2, -0.5, getLaneOffset() - m_LaneWidth / 2 + m_LaneWidth * (m_laneCount + 1), -0.5);
	const Line laneLine(-0.5,0, -0.5,Window::ClientRect().h);
	//m_rect.draw(Color(30, 30, 30));

	//レーン
	for (auto i = 0; i < m_laneCount; i++) {
		laneLine.movedBy(i * m_LaneWidth + getLaneOffset(), 0).draw(Palette::White);
	}

	laneLine.movedBy(m_laneCount * m_LaneWidth + getLaneOffset(), 0).draw(Color(50, 50, 50));

	Graphics2D::SetTransform(scrollMatrix);

	//小節
	double measureBeginY = 0;
	for (auto& measure : m_measures) {
		int lineCount = static_cast<int>(Ceil(m_division * measure->getLength()));

		double gridHeight = getBeatPerHeight() * 4 / m_division;
		double y = measureBeginY;

		divLine.movedBy(0, y).draw(Palette::Green);

		int emphasizeInterval = m_division / 4;

		for (int i = 1; i < lineCount; i++)
		{
			y -= gridHeight;
			divLine.movedBy(0,y).draw(i % emphasizeInterval == 0 ? Palette::White : Palette::Gray);
		}

		measure->BeginY = measureBeginY;
		measure->LastLineY = y;
		measureBeginY -= getBeatPerHeight() * 4 * measure->getLength();
		measure->EndY = measureBeginY;
	}

	Graphics2D::ClearTransform();

	divLine.movedBy(0,Window::ClientRect().h - m_gridOffset).draw(Palette::Red);

	Graphics2D::SetTransform(scrollMatrix);

	//小節番号
	for (size_t i = 0; i < m_measures.size(); ++i) {
		FontAsset(L"editor")(Pad(i, { 3, L'0' })).drawAt(getLaneOffset() - 70, m_measures[i]->BeginY);
	}

	//帯
	for (auto& note : m_playableNotes) {
		note->drawRibbon();
	}

	//ノート
	for (auto& note : m_playableNotes) {
		note->draw();
	}

	//ノート情報
	for (auto& note : m_playableNotes) {
		note->drawInfo();
	}

	//テンポ変更
	for (auto& note : m_changeTempos) {
		note->draw();
	}

	//小節選択
	Triangle(getLaneOffset() - 110, m_measures[m_selectedMeasure]->BeginY, 15, 90_deg).draw(Palette::Red);

	Println(L"Notes:", m_playableNotes.size());
}
