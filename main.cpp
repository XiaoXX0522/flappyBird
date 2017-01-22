#include <string>
#include <vector>
#include <sstream>
#include "cleanup.h"
#include "base.h"
#include "pipe.h"
#include "bird.h"

using namespace std;

int main(int, char**) {

	srand(time(NULL)); //�õ�ǰʱ���������������

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { //��ʼ��SDL�����⣬ʧ���򱨴�
		throw SDL_ERROR("init error: ");
	}
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) { //��ʼ��SDL_image�⣬ʧ���򱨴�
		throw SDL_ERROR("init img error: ");
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { //��ʼ��SDL_Mixer�⣬ʧ���򱨴�
		throw SDL_ERROR("init mixer error: ");
	}
	if (TTF_Init() != 0) { //��ʼ��SDL_TTF�⣬ʧ���򱨴�
		throw SDL_ERROR("init ttf error: ");
	}

	//����һ��SDL����
	SDL_Window * win = SDL_CreateWindow("flappy Mi", 300, 50, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == nullptr) { //������ʧ�����˳������׳��쳣
		SDL_Quit();
		throw SDL_ERROR("create window error: ");
	}

	//����SDL����Ⱦ��
	SDL_Renderer * ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr) { //������ʧ����ɾ��ǰһ���Ĵ��ڣ��˳����׳��쳣
		cleanup(win);
		SDL_Quit();
		throw SDL_ERROR("create renderer error: ");
	}
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND); //����Ⱦ����ģʽ����ΪBLEND������ʾ��͸��ͼ��

	SDL_Rect fill_screen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT }; //����ȫ���ڵľ��Σ�����֮������

	SDL_Texture * img_background = loadTexture(getResPath("background.png"), ren); //��ȡ����ͼƬ
	SDL_Texture * img_ground = loadTexture(getResPath("ground.png"), ren); //��ȡ����ͼƬ
	SDL_Texture * img_pipe = loadTexture(getResPath("pipe.png"), ren); //��ȡ�ܵ�ͼƬ
	SDL_Texture * img_board = loadTexture(getResPath("board.png"), ren); //��ȡ�Ʒְ�ͼƬ

	SDL_Texture * img_bird1 = loadTexture(getResPath("bird.png"), ren); //��ȡ��ͼƬ������״̬1
	SDL_Texture * img_bird2 = loadTexture(getResPath("bird2.png"), ren); //��ȡ��ͼƬ������״̬2
	SDL_Texture * img_bird3 = loadTexture(getResPath("bird3.png"), ren); //��ȡ��ͼƬ������״̬3
	SDL_Texture * img_bird_d = loadTexture(getResPath("bird_d.png"), ren); //��ȡײ���ϰ�����ͼƬ

	Mix_Chunk * snd_flap = loadSound(getResPath("flap.wav")); //��ȡ�ȳ����Ч�ļ�
	Mix_Chunk * snd_score = loadSound(getResPath("score.wav")); //��ȡ�÷ֵ���Ч�ļ�
	Mix_Chunk * snd_hit = loadSound(getResPath("hit.wav")); //��ȡײ�ϰ�����Ч�ļ�
	Mix_Chunk * snd_bounce = loadSound(getResPath("bounce.wav")); //��ȡ��������Ч�ļ�������bounce mode
	Mix_Chunk * snd_final = loadSound(getResPath("final.wav")); //��ȡ�Ʒְ����µ���Ч�ļ�

	TTF_Font * font = TTF_OpenFont(getResPath("Humor-Sans.ttf").c_str(), 50); //��ȡ�����ļ�
	if (font == nullptr) { throw SDL_ERROR("load font error: "); } //����ȡʧ���򱨴�
	
	stringstream ss; //��ʼ���ַ���������֮�󽫷���תΪ�ַ���
	SDL_Texture * txt_score = nullptr; //��ʼ���÷ֵ�������ͼ��ָ��
	SDL_Texture * txt_best = nullptr; //��ʼ����߷ֵ�������ͼ��ָ��
	SDL_Texture * txt_restart = createText("press any key to restart", font, { 0,0,0,255 }, ren); //����restart������ͼ
	SDL_SetTextureBlendMode(txt_restart, SDL_BLENDMODE_BLEND); //��restart��ͼ����Ϊ��͸�����Ա�ʵ��֮����Ч��
	SDL_Color WHITE = { 255,255,255,255 }; //������ɫ��ɫ
	SDL_Color BLACK = { 0,0,0,255 }; //������ɫ��ɫ
	
	bird myBird(ren, img_bird1, img_bird2, img_bird3, img_bird_d, snd_flap, snd_hit, snd_bounce); //��ʼ��С�񣬽���Ҫ���ز�ȫ������
	vector<pipe> pipeList = { pipe(ren, img_pipe, snd_score), pipe(ren, img_pipe, snd_score) }; //��ʼ���ܵ����б���Ϊһ����Ļ���ֻ�������ܵ�ͬʱ���֣�����Ϊ2���������ز�ȫ������
	SDL_Event e; //��ʼ���¼�����Ķ�������֮����������̲�����
	bool quit = false; //��־�˳����ı���
	unsigned ground_t = 0; //��־����λ�õı�������֮������û����ÿ֡��1��ʵ�ֵ�������
	unsigned dying_t = 0; //��־������֡���ı���������ʵ�����׺ͼƷְ����ȶ���Ч��
	int alpha, tWidth, tHeight; //���ֵ�͸���ȣ������

	while (!quit) { //quitΪ�ٵ�ʱ��ѭ����һֱ�������رհ�ť��esc���Ὣquit��Ϊ��

		if (myBird.state == FLYING) { //���Ƿ���״̬��ÿ֡�Ŀ�ͷ�׶���״̬��⣬�����Ƿ����ˮ�ܣ�������Ƿ�ײ����ˮ��
			if (ground_t % (600 / HORIZONTAL_SPEED) == 0) { //�������˶�����600/ˮƽ�ٶȣ������������������һ��ˮ��
				pipeList[0].init();
			}
			if (ground_t % (600 / HORIZONTAL_SPEED) == (600 / HORIZONTAL_SPEED / 2)) { //�������˶�����600/ˮƽ�ٶȣ��İ�������������ڶ���ˮ��
				pipeList[1].init();
			}
			for (auto&p : pipeList) { //��ÿ��ˮ�ܼ���Ƿ���ײ
				myBird.checkHit(p);
			}
		}

		while (SDL_PollEvent(&e)) { //�����¼�������ȡ�¼������е��¼����������
			if (e.type == SDL_QUIT) { //�����˹رռ���quit����Ϊ�棬ѭ�����������
				quit = true;
				break;
			}
			if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) { //���̻���갴�µĶ���
				if (e.key.keysym.sym == SDLK_ESCAPE) { //����esc��Ҳ���˳�
					quit = true;
					break;
				}
				if (myBird.state == DYING) { //�����Ѿ�������״̬�����κμ����ص���ʼ�׶�
					if (dying_t > 20) { //ֻ������20֮֡���������Ӧ������������̰�����������������
						myBird.init(); //��������Ϊ��ʼ״̬������
						pipe::grade = 0; //����������
						for (auto& p : pipeList) { //�����й����Ƴ���Ļ
							p.X = -p.width;
						}
					}
				}
				else { //����û��״̬�����κμ�����ʹ���ȳ�
					if (e.key.keysym.sym == SDLK_b && myBird.state==START) { //���Ǵ��ڿ�ʼ�׶β��Ұ���b���������bounce(����)ģʽ���������ӵ�������͵���ᵯ��
						myBird.bounce = true;
					}
					myBird.flap();
				}
			}
		}
		
		myBird.fall(); //ÿ֡ѭ���������亯�����������λ�úͽǶ�

		//���㲿�ֽ�����׼����ʼ��Ⱦ����
		SDL_RenderClear(ren); //����
		renderTexture(img_background, ren, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT * 7 / 8); //�����������������λ��

		for (auto& p : pipeList) { //������
			if (myBird.state == FLYING) { //���Ƿ���״̬���������ƶ�����ʵ�����ƶ����ٻ�
				p.move();
			}
			p.render(); //������
		}

		for (int i = 0; i < 35; ++i) { //�����棬��Ϊ���úܶ�С��ͼƴ�����ģ���Ҫѭ��35�Σ�ÿ�����һ����ͼ��λ�û�������������ground_t�������ڿ��Ƶ����ƶ�
			int pos = (i*SCREEN_WIDTH / 20 - ground_t*HORIZONTAL_SPEED) % (SCREEN_WIDTH * 21 / 20) - SCREEN_WIDTH / 20;
			renderTexture(img_ground, ren, pos, SCREEN_HEIGHT * 7 / 8, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 8);
		}

		myBird.render(); //����

		if (myBird.state == DYING) { //�����Ѿ�������״̬�����ײ����Ʒְ��restart��ʾ
			//����
			alpha = dying_t > 20 ? 85 : (255 * (30 - dying_t) / 30); //ͨ��alphaֵ����͸���ȣ���ȫ�ײ�͸���𽥽��͵���͸��
			SDL_SetRenderDrawColor(ren, 255, 255, 255, alpha); //�Ȱ���Ⱦ���Ļ�����ɫ��Ϊ��ɫ��͸����Ϊalpha
			SDL_RenderFillRect(ren, &fill_screen); //��������ĻͿɫ
			SDL_SetRenderDrawColor(ren, 0, 0, 0, 255); //�ѻ�����ɫ�Ļ�������ʵҲ���Բ��ģ�

			//���������ļƷְ�
			int board_y = dying_t < 25 ? -int((dying_t - 5) * (dying_t - 41))/2 : 160; //����dying_t���Ʒְ�������Ķ���������Ʒְ��������
			renderTexture(img_board, ren, (SCREEN_WIDTH - BOARD_WIDTH) / 2, board_y, BOARD_WIDTH, BOARD_HEIGHT); //���Ʒְ�

			//����������
			TTF_SetFontStyle(font, TTF_STYLE_NORMAL); //��������ģʽΪ�������Ǵ��壩

			ss << pipe::grade; //�����������ͣ������ַ��������Ա�ת��Ϊ�ַ���
			txt_score = createText(ss.str(), font, WHITE, ren);	 //����������������ͼ����ɫ
			SDL_QueryTexture(txt_score, NULL, NULL, &tWidth, &tHeight); //��ȡ������ͼ�ĳ���
			renderTexture(txt_score, ren, (SCREEN_WIDTH - tWidth) / 2, board_y+47); //������
			cleanup(txt_score); //�ͷŵ�������ͼ
			ss.str(""); //����ַ�����

			ss << pipe::highest; //����߷���ͬ������
			txt_best = createText(ss.str(), font, WHITE, ren); //������ͼ
			SDL_QueryTexture(txt_best, NULL, NULL, &tWidth, &tHeight); //��ȡ����
			renderTexture(txt_best, ren, (SCREEN_WIDTH - tWidth) / 2, board_y + 117); //����߷�
			cleanup(txt_best); //�ͷ���ͼ
			ss.str(""); //�����

			if (dying_t > 20) { //��50֡�󣬵�����ʾrestart��ʾ���֡�100֡������ѭ���䵭���ͨ��alphaʵ��
				alpha = dying_t < 70 ? 255 * (dying_t - 20) / 50 : 255 * (0.6 + 0.4 * cos(double(dying_t - 70) / 30));
				SDL_SetTextureAlphaMod(txt_restart, alpha); //������ͼ͸����
				renderTexture(txt_restart, ren, 50, 400, SCREEN_WIDTH - 100, 30); //��ͼ
			}

			if (dying_t == 10) { //��ʮ֡���żƷְ��������
				Mix_PlayChannel(-1, snd_final, 0);
			}

			dying_t++; //����ÿ֡ʱ���һ
		}
		else {
			TTF_SetFontStyle(font, TTF_STYLE_BOLD); //��������ģʽΪ���壬���ڷ���ʱ��ʾ����
			
			//ͨ���Ȼ�һ���ɫ�ٻ�һ���ɫ��ʵ���������
			ss << pipe::grade; //�ѷ������������Ա�ת��Ϊ�ַ���
			TTF_SetFontOutline(font, 1 ); //���������ԵΪ1���������ڱ�
			txt_score = createText(ss.str(), font, BLACK, ren); //������ɫ��ͼ���
			SDL_QueryTexture(txt_score, NULL, NULL, &tWidth, &tHeight); //��ȡ��ͼ����
			renderTexture(txt_score, ren, (SCREEN_WIDTH - tWidth) / 2, 100); //����ɫ�������
			cleanup(txt_score); //�ͷ���ͼ

			TTF_SetFontOutline(font, 0); //���������ԵΪ0������ɫ����
			txt_score = createText(ss.str(), font, WHITE, ren); //������ɫ������ͼ
			renderTexture(txt_score, ren, (SCREEN_WIDTH - tWidth) / 2, 100); //����ɫ�����ں�ɫ֮�ϣ�ʵ�����Ч��
			cleanup(txt_score); //�ͷ���ͼ

			ss.str(""); //����ַ�����
			dying_t = 0; //δ����ÿ������������ʱΪ0
			ground_t++; //���Ƶ���ǰ��
		}

		SDL_RenderPresent(ren); //��ʾͼ��
		SDL_Delay(1); //�ӳ�һ���룬����֡�ʹ���

	}
	//ѭ���������Ѿ��˳����ƺ���
	cleanup(txt_best, txt_restart, txt_score, font); //�ͷ������������ͼ
	cleanup(snd_final, snd_flap, snd_hit, snd_score); //�ͷ���Ч�ļ�
	cleanup(img_background, img_bird1, img_bird2, img_bird3, img_bird_d, img_pipe, img_ground, ren, win); //�ͷ���ͼ����Ⱦ���ʹ���
	TTF_Quit(); //�˳�TTFģ��
	IMG_Quit(); //�˳�ͼ��ģ��
	Mix_Quit(); //�˳���Чģ��
	SDL_Quit(); //�˳���ģ��

	return 0;
}