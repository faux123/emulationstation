#include "Sound.h"

#include "AudioManager.h"
#include "Log.h"
#include "Settings.h"
#include "ThemeData.h"

std::map< std::string, std::shared_ptr<Sound> > Sound::sMap;

std::shared_ptr<Sound> Sound::get(const std::string& path)
{
	auto it = sMap.find(path);
	if (it != sMap.cend()) {
		LOG(LogError) << "Sound::get (found in map)";
		return it->second;
	}

	std::shared_ptr<Sound> sound = std::shared_ptr<Sound>(new Sound(path));

	if (AudioManager::isInitialized())
	{
		AudioManager::getInstance()->registerSound(sound);
		sMap[path] = sound;
		LOG(LogError) << "Sound::get: " << path;
	}
	else
		LOG(LogError) << "Sound::get (AM not initialized)";

	return sound;
}

std::shared_ptr<Sound> Sound::getFromTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element)
{
	LOG(LogInfo) << " req sound [" << view << "." << element << "]";

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "sound");
	if (!elem || !elem->has("path"))
	{
		LOG(LogInfo) << "   (missing)";
		return get("");
	}

	return get(elem->get<std::string>("path"));
}

Sound::Sound(const std::string & path) : mSampleData(NULL), mPlayingChannel(-1)
{
	loadFile(path);
	LOG(LogError) << "Sound::new Sound instantiation";
}

Sound::~Sound()
{
	deinit();
}

void Sound::loadFile(const std::string & path)
{
	mPath = path;
	init();
	LOG(LogError) << "Sound::loadFile";
}

void Sound::init()
{
	deinit();

	if (!AudioManager::isInitialized())
	{
		LOG(LogError) << "Sound::init: AM not initialized";
		return;
	}

	if (mPath.empty() || !Utils::FileSystem::exists(mPath))
	{
		LOG(LogError) << "Sound::init: Empty path";
		return;
	}
	if (!Settings::getInstance()->getBool("EnableSounds"))
	{
		LOG(LogError) << "Sound::init: Sound not enabled";
		return;
	}

	//load wav file via SDL
	mSampleData = Mix_LoadWAV(mPath.c_str());
	if (mSampleData == nullptr)
	{
		LOG(LogError) << "Error loading sound \"" << mPath << "\"!\n" << "	" << SDL_GetError();
		return;
	}
	LOG(LogError) << "Sound::init";

}

void Sound::deinit()
{
	if (mSampleData == nullptr)
		return;

	stop();
	Mix_FreeChunk(mSampleData);
	mSampleData = nullptr;
	LOG(LogError) << "Sound::deinit";

}

void Sound::mixEnd_callback(int channel)
{
	//halt all channels
	LOG(LogError) << "Sound::mixEnd_callback";
	Mix_HaltChannel(-1);
	//Mix_CloseAudio();
}

void Sound::play()
{
	if (mSampleData == nullptr)
	{
		LOG(LogError) << "Sound::play(): nullptr";
		return;
	}

	if (!Settings::getInstance()->getBool("EnableSounds"))
		return;

	//Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	mPlayingChannel = Mix_PlayChannel(-1, mSampleData, 0);
	Mix_ChannelFinished(mixEnd_callback);
	LOG(LogError) << "Sound::play()";

}

bool Sound::isPlaying() const
{
	return (mPlayingChannel >= 0);
}

void Sound::stop()
{
	if (mPlayingChannel < 0)
		return;

	//Mix_HaltChannel(mPlayingChannel);
	mPlayingChannel = -1;
	LOG(LogError) << "Sound::stop()";
}
