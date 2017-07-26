#include "PlayableNote.h"


PlayableNote::PlayableNote(int tick, int lane, int channel, NoteType type) :
	Note(tick, lane)
{
	m_type = type;
	Channel = channel;
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

void PlayableNote::drawRibbon() const {
	if(ConnectNote!=nullptr)
		Line(m_position, ConnectNote->m_position).draw(30, Color(210, 255, 255));
}

NoteType PlayableNote::getNoteType() {
	return m_type;
}