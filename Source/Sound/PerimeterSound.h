#ifndef PERIMETER_SOUND_H
#define PERIMETER_SOUND_H

//Инициализация/деинициализация библиотеки
bool SNDInitSound(int mixChannels, int chunkSizeFactor);
void SNDReleaseSound();

void SNDEnableSound(bool enable);
void SNDEnableVoices(bool enable);
bool SNDIsVoicesEnabled();

void SNDSetSoundDirectory(const char* dir);
const std::string& SNDGetSoundDirectory();

void SNDSetLocDataDirectory(const char* dir);
void SNDSetBelligerentIndex(int idx);

//Работа с ошибками
void SNDEnableErrorLog(bool enable);

void SNDSetSoundVolume(float volume);//volume=0..1
void SNDSetVoiceVolume(float volume);//volume=0..1

//Грузит соответстивующую группу звуков в память 
//и позволяет к ним обращаться с помощью 
bool SNDScriptPrmEnableAll();

//Функции сделанные для вызова менюшек
//Вполне естественно, что внутри нежелательно вызывать 
//SNDScriptDisable, то есть можно, но только для тех скриптов, что не 
//исполдьзуются, иначе звуки не запустятся опыть

void SNDPausePush();//Остановить все звуки, c возможностью продолжения проигрывания 
void SNDPausePop();//Продолжить играть все остановленные звуки
int SNDGetPushLevel();//Возвращает уровень вложенности

void SNDStopAll();//Остановить все звуки

int SNDDeviceFrequency();
int SNDDeviceChannels();
SDL_AudioFormat SNDDeviceFormat();

static inline Uint16 SNDformatSampleSize(Uint16 format) {
    return (format & 0xFF) / 8;
}

///Get audio buffer time length (in us) given its size and current audio format
uint64_t SNDcomputeAudioLengthUS(uint64_t bytes);

///Get audio buffer time length (in ms) given its size and current audio format
static inline uint64_t SNDcomputeAudioLengthMS(uint64_t bytes) {
    return SNDcomputeAudioLengthUS(bytes) / 1000;
}

///Get audio buffer time length (in secs) given its size and current audio format
static inline double SNDcomputeAudioLengthS(size_t len) {
    return static_cast<double>(SNDcomputeAudioLengthUS(len)) / 1000000.0;
}

////////////////////////////3D/////////////////////////////////


class SND3DSound
{
	struct ScriptParam* script;
	int cur_buffer;
public:
	SND3DSound();
	~SND3DSound();
	bool Init(const char* name);

	bool Play(bool cycled=true);
	bool Stop();
	bool IsPlayed();
	
	void SetPos(const Vect3f& pos);//Обязательно вызвать до Play
	void SetVelocity(const Vect3f& velocity);
	void SetVolume(float vol);//0..1 учитывает volmin и volume


	//ScriptFrequency - установить относительную 
	bool SetFrequency(float frequency);//0..2 - 0 - минимальная, 1 - по умолчанию

	//SetFrequency - frequency=1..44100 Гц, оригинальная - 0
	void SetRealVolume(float vol);//0..1
protected:
	inline void AssertValid();
	void Destroy();
};

bool SND3DPlaySound(const char* name,
                    const Vect3f* pos,
                    const Vect3f* velocity=NULL//По умолчанию объект считается неподвижным
					);

class SND3DListener
{
protected:
	friend struct SNDOneBuffer;
	friend class SoftSound3D;
	friend class SND3DSound;

	Mat3f rotate,invrotate;
	Vect3f position;
	//MatXf mat;
	Vect3f velocity;

	//Дупликаты для software
	float s_distance_factor;
	float s_doppler_factor;
	float s_rolloff_factor;

	Vect3f front,top,right;

	float zmultiple;
public:
	SND3DListener();
	~SND3DListener();

	//Параметры изменяемые редко (скорее всего их менять и устанавливать не придётся никогда)
	//К тому-же они не работают (уж не знаю по какой причине)
	bool SetDistanceFactor(float);//1 - в метрах, 1000 - в километрах
	bool SetDopplerFactor(float);//0..10, по умолчанию 1
	bool SetRolloffFactor(float);

	//SetPos надо изменять каждый кадр
	bool SetPos(const MatXf& mat);

	Vect3f GetPos(){return position;};

	//SetVelocity - желательно изменять каждый кадр
	//иначе не будет смысла в SetDopplerFactor,SetRolloffFactor
	bool SetVelocity(const Vect3f& velocity);

	//Функция специально для Рубера
	//Что-бы расстояние по Z было меньше.
	//в реальном времени криво работает\
	//zmul=0..1
	void SetZMultiple(float zmul){zmultiple=zmul;};

	//Update - Вызывать после установки параметров (SetPos,...)
	//(один раз на кадр!)
	bool Update();
};

extern SND3DListener snd_listener;

////////////////////////////2D/////////////////////////////////

//volume : 0 - миниум, 1 - максиум
//pan : 0 - крайне левое положение, 0.5 - центр, +1 - крайне правое
//Для звуков, которые должны проиграться один раз
//и играться в одном месте
bool SND2DPlaySound(const char* name, float x=0.5f);
//Устанавливает параметры влияющие на SND2DPanByX
//width - ширина экрана, power - влияет на то, 
//насколько крайне правая точка будет звучать в левом наушнике
//power=1 - максимальное разнесение, power=0 - минимальное
void SND2DPanByX(float width,float power);

class SND2DSound
{
	struct ScriptParam* script;
	int cur_buffer;
public:
	SND2DSound();
	~SND2DSound();
	bool Init(const char* name);

	bool Play(bool cycled=true);
	bool Stop();
	bool IsPlayed() const;
	
	bool SetPos(float x);//Обязательно вызвать до Play
	bool SetFrequency(float frequency);//0..2 - 0 - минимальная, 1 - по умолчанию
	void SetVolume(float vol);//0..1

	////
	void SetRealVolume(float vol);//0..1
protected:
	inline void AssertValid();
	inline void Destroy();
};

#endif //PERIMETER_SOUND_H