#include "PlayableNote.h"


PlayableNote::PlayableNote(int tick, int startPos, int finishPos, int channel, NoteType type) :
	Note(tick)
{
	m_type = type;
	Channel = channel;
	StartPos = startPos;
	FinishPos = finishPos;
}

void PlayableNote::draw() const {
	switch (m_type)
	{
	case NoteType::Tap:
		TextureAsset(L"notes")(0, 0, 48, 48).drawAt(m_position);
		break;
	case NoteType::Long:
		TextureAsset(L"notes")(48, 0, 48, 48).drawAt(m_position);
		break;
	case NoteType::Slide:
		TextureAsset(L"notes")(96, 0, 48, 48).drawAt(m_position);
		break;
	case NoteType::LFlick:
		TextureAsset(L"notes")(0, 48, 60, 48).drawAt(m_position);
		break;
	case NoteType::RFlick:
		TextureAsset(L"notes")(60, 48, 60, 48).drawAt(m_position);
		break;
	}
}

void PlayableNote::drawInfo() const {
	FontAsset(L"editor")(StartPos + 1).drawAt(m_position, Palette::Black);
	FontAsset(L"editor")(Channel).drawAt(m_position-Point(25,25), Palette::Red);
}

void PlayableNote::drawRibbon() const {
	if (ConnectNote != nullptr)
		Line(m_position, ConnectNote->m_position).draw(30, Color(210, 255, 255));
}

NoteType PlayableNote::getNoteType() {
	return m_type;
}

void PlayableNote::setStartPos(int lane) {
	StartPos = lane;
}