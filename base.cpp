#include "base.h"

using namespace std;


SDL_ERROR::SDL_ERROR() :runtime_error(strlen(SDL_GetError()) ? SDL_GetError() : "Unknown Error.") {};
SDL_ERROR::SDL_ERROR(const char * msg) :runtime_error((string(msg) + SDL_GetError()).c_str()) {};
SDL_ERROR::SDL_ERROR(const string & msg) :runtime_error(msg + SDL_GetError()) {};

clock_t myClock() {
	return clock() * 1000 / CLOCKS_PER_SEC; //用于不同系统之间同步时间，保证返回的是毫秒单位
}

string getResPath(string path) {
	//return string(SDL_GetBasePath()) + "res\\" + path;
	return "res/" + path;  //用于不同不同系统之间找到相对路径
}

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren) {
	SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str()); //IMG_LoadTexture用于从图片文件生成贴图texture
	if (texture == nullptr) { //如果读取文件失败，texture是一个空指针，抛出异常
		throw SDL_ERROR("load texture error: ");
	}
	return texture;
}

Mix_Chunk* loadSound(const std::string &file) {
	Mix_Chunk *sound = Mix_LoadWAV(file.c_str()); //从文件读取音效，存入sound指针中
	if (sound == nullptr) { //如果读取文件失败，sound是一个空指针，抛出异常
		throw SDL_ERROR("load sound error: ");
	}
	return sound;
}

SDL_Texture* createText(const std::string &message, TTF_Font * font, SDL_Color color, SDL_Renderer *renderer) {
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);  //由于TTF包的渲染字体只能生成为surface
	if (surf == nullptr) { //失败则抛出异常
		throw SDL_ERROR("ttf render text error: ");
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf); //再从surface生成texture
	if (texture == nullptr) { //失败则抛出异常
		throw SDL_ERROR("create texture error: ");
	}
	SDL_FreeSurface(surf); //从内存中释放surface
	return texture;
}

void renderTexture(SDL_Texture * tex, SDL_Renderer * ren, int x, int y, int w, int h) {
	SDL_Rect rect; //矩形类，其中包含横坐标、纵坐标、宽度、高度
	rect.x = x;    //分别设置目标矩形rect的横坐标、纵坐标、宽度、高度
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_RenderCopy(ren, tex, NULL, &rect); //SDL_RenderCopy将指定贴图渲染到目标矩形中
}

void renderTexture(SDL_Texture * tex, SDL_Renderer * ren, int x, int y, int w, int h, double angle) {
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_RenderCopyEx(ren, tex, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
}

void renderTexture(SDL_Texture * tex, SDL_Renderer * ren, int x, int y, int w, int h, SDL_RendererFlip flip) { //flip是表明目标是否翻转的枚举变量，例如，当flip是SDL_FLIP_VERTICAL，贴图渲染时上下翻转
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_RenderCopyEx(ren, tex, NULL, &rect, NULL, NULL, flip);
}

void renderTexture(SDL_Texture * tex, SDL_Renderer * ren, int x, int y) {
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h); //SDL_QueryTexture用于获得贴图tex的宽度和高度
	renderTexture(tex, ren, x, y, w, h);
}

void renderTexture(SDL_Texture * tex, SDL_Renderer * ren, int x, int y, double angle) {
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture(tex, ren, x, y, w, h, angle);
}

void renderTexture(SDL_Texture * tex, SDL_Renderer * ren, int x, int y, SDL_RendererFlip flip) {
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture(tex, ren, x, y, w, h, flip);
}
