#include "Deleste.h"
#include "ChangeTempo.h"

Deleste::Deleste():
	m_music(SoundAsset(L"none"))
{
	for (size_t i = 0; i < 200; ++i) {
		getMeasures().push_back(std::make_shared<Measure>());
	}
}

Deleste::Deleste(String path):
	m_music(SoundAsset(L"none"))
{

	LOG(L"譜面の読み込みを開始します。");

	TextReader reader(path);
	String line;
	int lineNumber = 0;
	while (reader.readLine(line))
	{
		++lineNumber;
		Array<Match> matches;

		//長すぎる文字列がstd::regexで扱えないためエラー扱い。要改善。
		if (line.length > 300) {
			error(Format(L"[Error] {}行目: 文字列が長すぎます。"_fmt, lineNumber));
			continue;
		}

		if ((matches = Regex::Search(line, LR"(^(?:#|@)([A-Za-z]\S+)\s?(.*)$)")).size() != 0) {
			String tag = String(matches[0][1].str()).upper();
			auto args = String(matches[0][2].str()).split(L',');

			if (args.size() == 0) {
				continue;
			}
			else {
				bool continueFlag = false;
				for (auto& arg : args) {
					if (arg.isEmpty) {
						error(Format(L"[Error] {}行目: タグの引数が空です。[{}]"_fmt, lineNumber, tag));
						continueFlag = true;
					}
				}
				if (continueFlag)
					continue;
			}

			if (tag == L"TITLE") {
				Title = args[0];
			}
			else if (tag == L"LYRICIST") {
				Lyricist = args[0];
			}
			else if (tag == L"COMPOSER") {
				Composer = args[0];
			}
			else if (tag == L"BEATMAPPER" || tag == L"MAPPER" || tag == L"AUTHOR") {
				Mapper = args[0];
			}
			else if (tag == L"TEMPO" || tag == L"BPM") {
				Bpm = Parse<double>(args[0]);
			}
			else if (tag == L"LEVEL" || tag == L"LV") {
				Level = Parse<int>(args[0]);
			}
			else if (tag == L"DIFFICULTY" || tag == L"DIFFICULT") {
				Difficulty = args[0];
			}
			else if (tag == L"OFFSET") {
				Offset = Parse<int>(args[0]);
			}
			else if (tag == L"SONGOFFSET" || tag == L"MUSICOFFSET" || tag == L"BGMOFFSET") {
				SongOffset = Parse<int>(args[0]);
			}
			else if (tag == L"MOVOFFSET" || tag == L"MOVIEOFFSET") {
				MovieOffset = Parse<int>(args[0]);
			}
			else if (tag == L"SONG" || tag == L"MUSIC" || tag == L"BGM") {
				Song = args[0];
				const String fullPath = FileSystem::ParentPath(path) + Song;
				if (!FileSystem::Exists(fullPath)) {
					error(Format(L"[Error] {}行目: 音源が見つかりません。[{}]"_fmt, lineNumber, Song));
					continue;
				}
				m_music = Sound(fullPath);
			}
			else if (tag == L"LYRICS" || tag == L"LYRIC") {
				Lyrics = args[0];
			}
			else if (tag == L"BACKGROUND" || tag == L"BACK" || tag == L"BG") {
				Background = args[0];
			}
			else if (tag == L"SONGVOLUME" || tag == L"SONGVOL" || tag == L"MUSICVOLUME" || tag == L"MUSICVOL" || tag == L"BGMVOLUME" || tag == L"BGMVOL") {
				SongVolume = Parse<int>(args[0]);
			}
			else if (tag == L"SEVOLUME" || tag == L"SEVOL") {
				SEVolume = Parse<int>(args[0]);
			}
			else if (tag == L"MEASURE" || tag == L"MEAS" || tag == L"MEA") {
				double measure = Parse<double>(args[0]);
				String length = args[1];
				if (Fraction(measure) != 0) {
					error(Format(L"[Error] {}行目: 小節数に小数は使用できません。[{}]"_fmt, lineNumber, measure));
					continue;
				}
				existMeasureCheck(static_cast<size_t>(measure));
				auto nums = length.split(L'/');
				if (nums.size() >= 2) {
					int numer = Parse<int>(nums[0]);
					int denom = Parse<int>(nums[1]);
					if (numer <= 0 || denom <= 0) {
						error(Format(L"[Error] {}行目: 小節の長さは0以下に設定出来ません。[{}/{}]"_fmt, lineNumber, numer, denom));
						continue;
					}
					else if (denom != 4 && denom != 8 && denom != 16 && denom != 32 && denom != 64) {
						error(Format(L"[Error] {}行目: 分母は4,8,16,32,64のみが使用出来ます。[{}/{}]"_fmt, lineNumber, numer, denom));
						continue;
					}
					m_measures[static_cast<size_t>(measure)]->Rhythm = std::make_shared<Rhythm>(numer, denom);
				}
				else {
					int numer;
					int denom;
					rationalize(Parse<double>(nums[0]), &numer, &denom);
					m_measures[static_cast<size_t>(measure)]->Rhythm = std::make_shared<Rhythm>(numer, denom);
				}
			}
			else if (tag == L"CHANGETEMPO" || tag == L"CHANGEBPM") {
				double measure = Parse<double>(args[0]);
				double tempo = Parse<double>(args[1]);
				existMeasureCheck(static_cast<size_t>(Floor(measure)));
				m_measures[static_cast<size_t>(Floor(measure))]->getNotes().push_back(std::make_shared<ChangeTempo>(static_cast<int>(Fraction(measure) * 768), tempo));
			}
			else if (tag == L"ATTRIBUTE") {
				Attibute = args[0];
			}
			else if (tag == L"BRIGHTNESS" || tag == L"BRIGHT") {
				Brightness = Parse<int>(args[0]);
			}
			else {
				error(Format(L"[Error] {}行目: 非対応のタグです。[{}]"_fmt, lineNumber, tag));
				continue;
			}
		}
		else if ((matches = Regex::Search(line, LR"(^#([0-9]+),([0-9]+):([0-9]+):?([0-9]*):?([0-9]*)$)")).size() != 0) {
			int channel = FromString<int>(matches[0][1].str(), 10);
			double measure = FromString<int>(matches[0][2].str(), 10);
			if (Fraction(measure) != 0) {
				error(Format(L"[Error] {}行目: 小節数に小数は使用できません。[{}]"_fmt, lineNumber, measure));
				continue;
			}
			existMeasureCheck(static_cast<size_t>(Floor(measure)));
			String notesInfo = matches[0][3].str();
			String startInfo = matches[0][4].str();
			String finishInfo = matches[0][5].str();
			int j = 0;
			for (size_t i = 0; i < notesInfo.length; ++i) {
				int type = Parse<int>(notesInfo.substr(i, 1));
				if (1 <= type && type <= 5)
				{
					int startPos = startInfo.isEmpty ? 3 : Parse<int>(startInfo.substr(j, 1));
					int finishPos = finishInfo.isEmpty ? startPos : Parse<int>(finishInfo.substr(j, 1));
					m_measures[static_cast<size_t>(Floor(measure))]->getNotes().push_back(std::make_shared<PlayableNote>(static_cast<double>(i) / notesInfo.length, startPos - 1, finishPos - 1, channel, static_cast<NoteType>(type)));
					++j;
				}
				else if (5 < type) {
					error(Format(L"[Error] {}行目: 無効なノートの種類です。[{}]"_fmt, lineNumber, type));
					++j;
				}
			}
		}
	}

	Measure::updateMeasureState(m_measures);

	for (auto& measure : m_measures) {
		measure->culcTick();
	}

	LOG(L"譜面を読み込みました。");
}

bool Deleste::hasError() {
	return m_errorMessages.size() != 0;
}

std::vector<String> &Deleste::getErrorMessages() {
	return m_errorMessages;
}

void Deleste::error(const String &message) {
	LOG(message);
	m_errorMessages.push_back(message);
}


void Deleste::save(const String& path) {
	m_savePath = path;
	TextWriter writer(path);

	if (!Title.isEmpty) writer.writeln(L"#Title ", Title);
	if (!Lyricist.isEmpty) writer.writeln(L"#Lyricist ", Lyricist);
	if (!Composer.isEmpty) writer.writeln(L"#Composer ", Composer);
	if (!Mapper.isEmpty) writer.writeln(L"#Mapper ", Mapper);
	if (!Background.isEmpty) writer.writeln(L"#Background ", Background);
	if (!Song.isEmpty) writer.writeln(L"#Song ", Song);
	if (!Lyrics.isEmpty) writer.writeln(L"#Lyrics ", Lyrics);
	/*
	if (!Lyricist.isEmpty) writer.writeln(L"#Lyricist ", Lyricist);
	if (!Lyricist.isEmpty) writer.writeln(L"#Lyricist ", Lyricist);
	if (!Lyricist.isEmpty) writer.writeln(L"#Lyricist ", Lyricist);*/

	for (size_t i = 0; i < m_measures.size(); ++i) {
		if (m_measures[i]->Rhythm != nullptr) {
			writer.writeln(L"#Measure {},{}"_fmt, i, m_measures[i]->Rhythm->toString());
		}

		for (auto& note : m_measures[i]->getNotes()) {
			if (typeid(*note) == typeid(ChangeTempo)) {
				auto changeTempo = std::dynamic_pointer_cast<ChangeTempo>(note);
				writer.writeln(L"#ChangeBPM {},{}"_fmt, i + static_cast<double>(changeTempo->Tick) / 768, changeTempo->getTempo());
			}
		}

		std::vector<std::shared_ptr<DelesteLine>> lines;

		for (auto& note : m_measures[i]->getNotes()) {
			if (typeid(*note) == typeid(PlayableNote)) {
				auto playableNote = std::dynamic_pointer_cast<PlayableNote>(note);
				
				auto insertLine = std::find_if(lines.begin(), lines.end(), [&](std::shared_ptr<DelesteLine>& x) {
					return x->getChannel() == playableNote->Channel
						&& x->getNotes().end() == std::find_if(x->getNotes().begin(), x->getNotes().end(), [&](std::shared_ptr<PlayableNote>& y) {return y->Tick == playableNote->Tick; });
				});
				if (insertLine != lines.end()) {
					(*insertLine)->getNotes().push_back(playableNote);
				}
				else {
					lines.push_back(std::make_shared<DelesteLine>(i, playableNote->Channel, m_measures[i]->getLengthTick(), playableNote));
				}
			}
		}

		for (auto& line : lines) {
			LOG(line->toString());
			writer.writeln(line->toString());
		}
	}
}

void Deleste::existMeasureCheck(size_t measure) {
	if (m_measures.size() <= measure) {
		while ((m_measures.size() - 1) != measure) {
			m_measures.push_back(std::make_shared<Measure>());
		}
	}
}

std::vector<std::shared_ptr<Measure>>& Deleste::getMeasures() {
	return m_measures;
}

const Sound& Deleste::getMusic() {
	return m_music;
}

const String& Deleste::getSavePath() {
	return m_savePath;
}

DelesteLine::DelesteLine(int measure, int channel, int length, std::shared_ptr<PlayableNote> note) :
	m_measure(measure),
	m_channel(channel),
	m_lengthTick(length)
{
	m_notes.push_back(note);
}

std::vector<std::shared_ptr<PlayableNote>>& DelesteLine::getNotes() {
	return m_notes;
}

int DelesteLine::getChannel() {
	return m_channel;
}

const String DelesteLine::toString() {
	String notes;
	String startPos;
	String finishPos;
	int charPerTick = 768;

	for (auto& note : m_notes) {
		charPerTick = Util::gcd(note->Tick, charPerTick);
	}

	int division = m_lengthTick / charPerTick;

	for (int i = 0; i < division; ++i) {
		const auto note = std::find_if(m_notes.begin(), m_notes.end(), [&](std::shared_ptr<PlayableNote>& x) {return x->Tick == charPerTick*i; });
		if (note != m_notes.end()) {
			notes.append(Format(static_cast<int>((*note)->getNoteType())));
			startPos.append(Format((*note)->StartPos + 1));
			finishPos.append(Format((*note)->FinishPos + 1));
		}
		else {
			notes.append(L"0");
		}
	}
	return Format(L"#{},{}:{}:{}:{}"_fmt, m_channel, m_measure, notes, startPos, finishPos);
}