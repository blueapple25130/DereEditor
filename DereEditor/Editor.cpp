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
				note->setPosition(getLaneOffset() + dynamic_cast<PlayableNote*>(note.get())->Lane * m_LaneWidth, -currentNoteTick / 192 * getBeatPerHeight());
			}
			else if (typeid(*note) == typeid(ChangeTempo)) {
				note->setPosition(512, -currentNoteTick / 192 * getBeatPerHeight());
				currentTempo = dynamic_cast<ChangeTempo*>(note.get())->getTempo();
				//TempoChangeLeap.Add(note);
			}
		}
		m_notes.insert(m_notes.end(), measure->getNotes().begin(), measure->getNotes().end());
		++measureIndex;

		currentMeasureRealTick += measure->getRealTick();
	}

	// TODO:�m�[�g�Ԃ̐ڑ�������
	// TODO:�e���|���l�������X�N���[��������

}

double Editor::getBeatPerHeight() {
	return m_zoom * BeatPerHeight;
}

//TODO:�σE�B���h�E�T�C�Y�Ή�

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
	//GUI������

	m_gui.addln(GUIText::Create(L"�O���b�h"));
	m_gui.add(GUIText::Create(L"�Y�[��"));
	m_gui.add(L"zoom", GUISlider::Create(0.2, 2.0, 1.0));
	m_gui.addln(L"zoom-val", GUIText::Create(L"100%"));

	m_gui.add(L"hr", GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"�e���|�ύX"));
	m_gui.add(GUIText::Create(L"BPM"));
	m_gui.addln(L"tempo", GUITextField::Create(none));

	m_gui.add(L"hr", GUIHorizontalLine::Create(1));

	m_gui.addln(GUIText::Create(L"����"));
	m_gui.add(L"add-measure", GUIButton::Create(L"�ǉ�"));
	m_gui.addln(L"remove-measure", GUIButton::Create(L"�폜"));
	m_gui.add(L"tf1", GUITextField::Create(none));
	m_gui.add(L"set-measure", GUIButton::Create(L"�ύX"));
	m_gui.setPos(m_rect.tr);

	for (int i = 0; i < 200; ++i) {
		m_measures.push_back(std::make_shared<Measure>());
	}
}

void Editor::update() {
	//GUI�֘A
	if (m_gui.slider(L"zoom").hasChanged) {
		m_zoom = m_gui.slider(L"zoom").value;
		m_gui.text(L"zoom-val").text = Format(static_cast<int>(m_zoom * 100), L"%");
		updateNoteState();
	}

	//TODO:�L�[�V���[�g�J�b�g

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

			//���q���l�����Ĕ��肷��
			//(�����������ߊJ�n���̂�?���߂̒���:���ߊJ�n��-�O���b�h�̍���/2)�ȏ�A���ߊJ�n���ƑO�̏��߂̍Ō�̕������̒��Ԗ���
			if (y - (lineCount == 1 ? (y - (*it)->EndY) / 2 : gridHeightHalf) <= mousePos.y && mousePos.y < y + (it == m_measures.begin() ? gridHeightHalf : ((*(it - 1))->LastLineY - y) / 2))
			{
				setOrRemoveNote((*it)->getNotes(), 0, lane);
			}
			y -= gridHeightHalf * 2;

			//����
			//�����������ߊJ�n���݂̂̏ꍇ�A�X�L�b�v
			//������-�O���b�h�̍���/2�̈ȏ�A������+�O���b�h�̍���/2�����Ŕ���
			for (int i = 1; i < lineCount - 1; i++)
			{
				if (y - gridHeightHalf <= mousePos.y && mousePos.y < y + gridHeightHalf)
				{
					int tick = 192 / (m_division / 4) * i;
					setOrRemoveNote((*it)->getNotes(), tick, lane);
				}
				y -= gridHeightHalf * 2;
			}
			//�Ō�
			//�����������ߊJ�n���݂̂̏ꍇ�A�X�L�b�v
			//�������Ə��ߏI��(���̏��߂̊J�n��)�̒��Ԉȏ�A������+�O���b�h�̍���/2�����Ŕ���
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

	//���[��
	for (auto i = 0; i < 5; i++) {
		Rect(i * m_LaneWidth + getLaneOffset(), Window::ClientRect().y, 1, Window::ClientRect().h).draw(Palette::White);
	}

	Rect(5 * m_LaneWidth + getLaneOffset() + 20, Window::ClientRect().y, 1, Window::ClientRect().h).draw(Color(50, 50, 50));

	Graphics2D::SetTransform(Mat3x2::Identity().translate(0, Window::ClientRect().h));

	//����
	double measureBeginY = 0;
	for (auto& measure : m_measures) {
		int lineCount = static_cast<int>(Ceil(m_division * measure->getLength()));

		double gridHeight = getBeatPerHeight() * 4 / m_division;
		double y = measureBeginY;

		Rect(getLaneOffset() - m_LaneWidth / 2, y, 462, 1).draw(Palette::Green);

		int emphasizeInterval = m_division / 4;

		for (int i = 1; i < lineCount; i++)
		{
			y -= gridHeight;
			Rect(getLaneOffset() - m_LaneWidth / 2, y, 462, 1).draw(i % emphasizeInterval == 0 ? Palette::White : Palette::Gray);
		}

		measure->BeginY = measureBeginY;
		measure->LastLineY = y;
		measureBeginY -= getBeatPerHeight() * 4 * measure->getLength();
		measure->EndY = measureBeginY;
	}

	//���ߔԍ�
	for (int i = 0; i < m_measures.size(); ++i) {
		FontAsset(L"editor")(Pad(i, { 3, L'0' })).drawAt(getLaneOffset() - 70, m_measures[i]->BeginY);
	}

	//�m�[�g
	for (auto& note : m_notes) {
		if (typeid(*note) == typeid(PlayableNote))
			note->draw();
	}

	//�e���|�ύX
	for (auto& note : m_notes) {
		if (typeid(*note) == typeid(ChangeTempo))
			note->draw();
	}

	//���ߑI��
	Triangle(getLaneOffset() - 110, m_measures[m_selectedMeasure]->BeginY, 15, 90_deg).draw(Palette::Red);

	Println(L"Notes:", m_notes.size());
}
