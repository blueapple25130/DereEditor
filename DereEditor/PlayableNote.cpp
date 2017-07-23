#include "PlayableNote.h"


PlayableNote::PlayableNote(int tick, int lane, NoteType type) :
	Note(tick, lane)
{
	m_type = type;
}

void PlayableNote::draw() const {
	//NoteType::Slide < m_type ? Point(48 * static_cast<int>(m_type), 0) : Point(60 * (static_cast<int>(m_type) - 2), 48)
	TextureAsset(L"notes")(0, 0, 48, 48).drawAt(m_position);
}