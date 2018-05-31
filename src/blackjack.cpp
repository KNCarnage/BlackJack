#include "blackjack.h"

#define ACE 0
#define TWO 1
#define THREE 2
#define FOUR 3
#define FIVE 4
#define SIX 5
#define SEVEN 6
#define EIGHT 7
#define NINE 8
#define TEN 9
#define JACK 10
#define QUEEN 11
#define KING 12

#define CLUB 0
#define DIAMOND 1
#define SPADE 2
#define HEART 3

#define mb_info		0
#define mb_ok		1
#define mb_yes_no	2

Resources gameResources;
float		gameScale;
const float		buttonScale = 1.3f;
const float		plusminusScale = 0.75f;
const float		backgroundScale = 1.4f;
const float		TextScale = 1.4f;

template <class T> class Data_Buffer 
{
public:
	Data_Buffer()
	{

	}

	void Init(size_t size)
	{
		buf_ = std::unique_ptr<T[]>(new T[size]);
		size_ = size;
	}

	void Add(T item)
	{
		buf_[head_] = item;
		head_ = (head_ + 1) % size_;

		if (head_ == tail_)
			tail_ = (tail_ + 1) % size_;
		return;
	}

	T Get(bool del)
	{
		if (isEmpty())
			return T();
		
		auto val = buf_[tail_];

		if (del)
			tail_ = (tail_ + 1) % size_;
		
		return val;
	}

	void Reset(void)
	{
		head_ = tail_;
	}

	bool isEmpty(void)
	{
		return head_ == tail_;
	}

	bool isFull(void)
	{
		return ((head_ + 1) % size_) == tail_;
	}

	size_t getSize(void)
	{
		return size_ - 1;
	}


private:
	std::unique_ptr<T[]> buf_;
	size_t head_ = 0;
	size_t tail_ = 0;
	size_t size_;
};

class MainDeck : public Actor
{
public:
	ResAnim* _deck;
	ResAnim* _back;
	spTextField _text;
	spSprite _button;

	typedef struct
	{
		int		valorap;
		int		used;
		int		inuse[7];
		int		valor;
		float	angle;
		Vector2 Pos;
		Vector2 Dist;
		spSprite card;
	} struct_mano;

	typedef struct
	{
		Point			pCard;
		Vector2			EndPos;
		float			angle;
		bool			show;
		bool			bFlip;
		int				iValor;
		int				CurrentNum;
		struct_mano*	Mano;
		spSprite card;
	} struct_datacards;



	bool bInuse;
	const bool RiseBet = true;
	const bool LowerBet = false;

	struct_datacards Shuf[312];
	struct_datacards Mazo[312];
	struct_mano		Player[4];
	struct_mano		Croupier;
	struct_mano		CardDeck[6];
	spTextField croupier_text;
	spTextField money_text;
	spTextField player_text[4];

	Vector2 CardBaseSize;
	float scaleFactor;
	Vector2 CardSep;
	int used[312];
	int apuesta, money, apinicial;
	int mazos;
	int started, seguro, mazoact;
	int crdsplyed;

	bool CheckRules = false;
	bool CheckOferta = false;

	Data_Buffer<struct_datacards *> SendBuffer;

	MainDeck()
	{
		spSprite background;
		int CardW, CardH;

		SendBuffer.Init(50);

		_deck = gameResources.getResAnim("cards");
		_back = gameResources.getResAnim("cardsback");

		CardW = _deck->getAttribute("cardwidth").as_int(0);
		CardH = _deck->getAttribute("cardheight").as_int(0);

		scaleFactor = gameScale *0.8f;
		CardSep.x = 15 * gameScale;
		CardSep.y = CardSep.x;

		CardBaseSize.x = (float)(int)(CardW*scaleFactor);
		CardBaseSize.y = (float)(int)(CardH*scaleFactor);

		background = new Sprite();
		background->setResAnim(gameResources.getResAnim("blackjack"));
		background->setAnchor(0.5f, 0.5f);
		background->setScale(gameScale * backgroundScale);
		background->setX(getStage()->getWidth() / 2);
		background->setY(2 * CardBaseSize.y - CardSep.y);
		addChild(background);

		CardDeck[0].Pos.x = 2 * CardBaseSize.x;
		CardDeck[0].Pos.y = CardSep.y + CardBaseSize.y / 2;
		
		CardDeck[0].Dist.x = CardSep.x / 2;
		CardDeck[0].Dist.y = CardSep.y / 2;

		Croupier.Pos.x = getStage()->getWidth() / 2 - CardBaseSize.x / 2;
		Croupier.Pos.y = CardSep.y + CardBaseSize.y / 2;

		Croupier.Dist.x = 1.5f*CardSep.x;
		Croupier.Dist.y = 0;

		Player[0].Pos.x = getStage()->getWidth() / 4 - CardBaseSize.x / 2;
		Player[0].Pos.y = getStage()->getHeight() / 2.5f + CardBaseSize.y / 3;

		Player[1].Pos.x = getStage()->getWidth() / 2.5f - CardBaseSize.x / 2;
		Player[1].Pos.y = getStage()->getHeight() / 2.2f + CardBaseSize.y;

		Player[2].Pos.x = getStage()->getWidth() - (getStage()->getWidth() / 2.5f) + CardBaseSize.x / 2;;
		Player[2].Pos.y = getStage()->getHeight() / 2.2f + CardBaseSize.y;

		Player[3].Pos.x = 3* getStage()->getWidth() / 4 + CardBaseSize.x / 2;
		Player[3].Pos.y = getStage()->getHeight() / 2.5f + CardBaseSize.y / 3;

		Player[0].Dist.x = CardSep.x;
		Player[0].Dist.y = CardSep.y;

		Player[1].Dist.x = 1.3f*CardSep.x;
		Player[1].Dist.y = CardSep.y/2;

		Player[2].Dist.x = 1.3f*CardSep.x;
		Player[2].Dist.y = -CardSep.y / 2;

		Player[3].Dist.x = CardSep.x;
		Player[3].Dist.y = -CardSep.y;

		Croupier.angle = 0;
		Player[0].angle = 50;
		Player[1].angle = 25;
		Player[2].angle = -25;
		Player[3].angle = -50;

		CardsCtr();
		ShowBaseCards();
		CardsShf();
		AddBetButtons();
		AddActionButtons();
		DisableActionButtons();
		return;
	}

	void CreateDeck(void)
	{
		spSprite card;
		int i;

		for (i = 1; i < 6; i++)
		{
			card = new Sprite();
			CardDeck[i].card = card;
			addChild(CardDeck[i].card);
			card->setScale(scaleFactor);
			card->setAnchor(0.5f, 0.5f);
			card->setAnimFrame((_back), 1);
			CardDeck[i].Pos.x = CardDeck[0].Pos.x + i*CardDeck[0].Dist.x;
			CardDeck[i].Pos.y = CardDeck[0].Pos.y + i*CardDeck[0].Dist.y;
			card->setX(CardDeck[i].Pos.x);
			card->setY(CardDeck[i].Pos.y);
		}

		return;
	}
	void ShowBaseCards(void)
	{
		spSprite card;
		TextStyle style;
		int i;

		CardDeck[0].card = new Sprite();
		addChild(CardDeck[0].card);
		CardDeck[0].card->setScale(scaleFactor);
		CardDeck[0].card->setAnchor(0.5f, 0.5f);
		CardDeck[0].card->setAnimFrame((_back), 1);
		CardDeck[0].card->setX(CardDeck[0].Pos.x);
		CardDeck[0].card->setY(CardDeck[0].Pos.y);
	
		Croupier.card = new Sprite();
		addChild(Croupier.card);
		Croupier.card->setScale(scaleFactor);
		Croupier.card->setAnchor(0.5f, 0.5f);
		Croupier.card->setAnimFrame((_back), 2);
		Croupier.card->setX(Croupier.Pos.x);
		Croupier.card->setY(Croupier.Pos.y);

		croupier_text = new TextField();
		croupier_text->setScale(Croupier.card->getScale() * TextScale);
//		croupier_text->setScale(1/scaleFactor*0.75f);
		croupier_text->setName("croupier_text");
		croupier_text->setY(Croupier.card->getHeight() + 2 * CardSep.y);

//		croupier_text->setY(CardBaseSize.y + 5 * CardSep.y);
		style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignLeft().alignBaseline();
		croupier_text->setStyle(style);
		croupier_text->attachTo(Croupier.card);

		for (i = 0; i < 4; i++)
		{
			card = new Sprite();
			Player[i].card = card;
			addChild(Player[i].card);
			card->setScale(scaleFactor);
			card->setAnchor(0.5f, 0.5f);
			card->setAnimFrame((_back), 2);
			card->setRotationDegrees((float)Player[i].angle);
			card->setX(Player[i].Pos.x);
			card->setY(Player[i].Pos.y);

			player_text[i] = new TextField();
			player_text[i]->setScale(card->getScale() * TextScale);
//			player_text[i]->setScale(1 / scaleFactor*0.75f);
			style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignLeft().alignMiddleV();
			player_text[i]->setStyle(style);
			switch (i)
			{	
				default:
				case 0:
					player_text[i]->setName("player_text_0");
				break;
				case 1:
					player_text[i]->setName("player_text_1");
					break;
				case 2:
					player_text[i]->setName("player_text_2");
					break;
				case 3:
					player_text[i]->setName("player_text_3");
				break;
			}
			player_text[i]->setY(card->getHeight() + 2 * CardSep.y);
//			player_text[i]->setY(CardBaseSize.y + 7 * CardSep.y);
			player_text[i]->attachTo(card);

		}
		return;
	}

	void AddBetButtons(void)
	{
		spTextField button_text, BetText;
		spInputText _input;
		spSprite bet,minus,plus;
		TextStyle style;
		BetText = new TextField;
		char text[255];
		
		safe_sprintf(text, "Money: %d", money);
		money_text = new TextField();
		money_text->setName("money_text");
		money_text->setText(text);
		money_text->setScale(gameScale * buttonScale);
		money_text->setX(7 * (getStage()->getWidth() / 8));
		money_text->setY(CardBaseSize.y / 4);
		style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
		money_text->setStyle(style);
		addChild(money_text);

		safe_sprintf(text, "%d", apinicial);
		BetText = new TextField();
		BetText->setName("BetText");
		BetText->setText(text);
		BetText->setScale(gameScale * buttonScale);
		BetText->setX(7 * (getStage()->getWidth() / 8));
		BetText->setY(CardBaseSize.y/2);
		style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
		BetText->setStyle(style);
		addChild(BetText);

		plus = new Sprite();
		plus->setName("button_plus");
		plus->setResAnim(gameResources.getResAnim("plusminus"));
		plus->setAnchor(0.5f, 0.5f);
		plus->setScale(gameScale * plusminusScale);
		plus->setX(BetText->getX() - CardBaseSize.x);
		plus->setY(CardBaseSize.y / 2 + CardSep.y);
		plus->setUserData(&RiseBet);
		plus->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ChangeBet));
		addChild(plus);

		minus = new Sprite();
		minus->setName("button_minus");
		minus->setResAnim(gameResources.getResAnim("plusminus"));
		minus->setAnimFrame(gameResources.getResAnim("plusminus"), 1);
		minus->setAnchor(0.5f, 0.5f);
		minus->setScale(gameScale * plusminusScale);
		minus->setX(BetText->getX() + CardBaseSize.x);
		minus->setY(CardBaseSize.y / 2 + CardSep.y);
		minus->setUserData(&LowerBet);
		minus->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ChangeBet));
		addChild(minus);


		bet = new Sprite();
		bet->setName("button_bet");
		bet->setResAnim(gameResources.getResAnim("button"));
		bet->setAnchor(0.5f, 0.5f);
		bet->setScale(gameScale * buttonScale);
		bet->setX(7 * (getStage()->getWidth() / 8));
		bet->setY(getStage()->getHeight() / 5);
		bet->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		bet->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		bet->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::CheckBet));
		addChild(bet);

		button_text = new TextField();
		button_text->setName("bet_text");
		button_text->setText("BET");
		button_text->attachTo(bet);
		button_text->setPosition(bet->getSize() / 2);
		style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
		button_text->setStyle(style);
		return;
	}

	void AddActionButtons(void)
	{
		spTextField button_text;
		spSprite button;
		int i;

		for (i = 0; i < 4; i++)
		{
			button = new Sprite();
			button->setResAnim(gameResources.getResAnim("button"));
			button->setAnchor(0.5f, 0.5f);
			button->setScale(gameScale * buttonScale);
			button->setX(i*(getStage()->getWidth() / 4) + (getStage()->getWidth() / 8));
			button->setY(getStage()->getHeight() - CardBaseSize.y + button->getHeight());
			switch (i)
			{
				default:
				case 0:
					button->setName("button_hit");
				break;
				case 1:
					button->setName("button_stand");
				break;
				case 2:
					button->setName("button_double");
				break;
				case 3:
					button->setName("button_split");
				break;
			}
			addChild(button);

			button_text = new TextField();
			button_text->setName("text");
			switch (i)
			{
				default:
				case 0:
					button_text->setText("HIT");
				break;
				case 1:
					button_text->setText("STAND");
				break;
				case 2:
					button_text->setText("DOUBLE");
				break;
				case 3:
					button_text->setText("SPLIT");
				break;
			}
			button_text->attachTo(button);
			button_text->setPosition(button->getSize() / 2);
			TextStyle style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
			button_text->setStyle(style);
		}
		return;
	}

	void CardsCtr(void)
	{
		int i, cara, pinta;

		cara = pinta = 0;
		apuesta = 0;
		money = 5000;
		apinicial = 300;

		for (i = 0; i < 312; i++)
		{
			Shuf[i].iValor = i;
			Shuf[i].pCard.x = cara;
			Shuf[i].pCard.y = pinta;
			cara = cara + 1;

			if (cara > 12)
			{
				cara = 0;
				pinta = pinta + 1;
			}
			if (pinta > 3)
				pinta = 0;
		}
	}

	void CardsShf(void)
	{
		time_t t1;
		int select, found, i;

		crdsplyed = 0;
		CreateDeck();
		(void)time(&t1);
		srand((long)t1);

		for (i = 0; i < 312; i++)
		{
			used[i] = 0;
		}
		for (i = 0; i < 312; i++)
		{
			found = 0;
			while (!found)
			{
				select = rand() % 312;
				if (used[select] == 0)
				{
					used[select] = 1;
					found = 1;
					Mazo[i].angle = 0;
					Mazo[i].bFlip = false;
					Mazo[i].card = NULL;
					Mazo[i].CurrentNum = 0;
					Mazo[i].EndPos = Vector2(0, 0);
					Mazo[i].iValor = Shuf[select].iValor;
					Mazo[i].Mano = NULL;
					Mazo[i].pCard = Shuf[select].pCard;
					Mazo[i].show = false;
				}
			}
		}
	}

	void cardShowText(struct_datacards* dataCard)
	{
		char	text[255];
		int		valor;
		spTextField text_info;

		if (dataCard->Mano == &Croupier)
		{
			text_info = this->getDescendantT<TextField>("croupier_text", ep_ignore_error);
			valor = CheckValor(dataCard->Mano, dataCard->CurrentNum);
			safe_sprintf(text, "Dealer: %d", valor);
			text_info->setText(text);
		}
		if (dataCard->Mano == &Player[0])
		{
			text_info = this->getDescendantT<TextField>("player_text_0", ep_ignore_error);
			valor = CheckValor(dataCard->Mano, dataCard->CurrentNum);
			safe_sprintf(text, "Mano: %d\nBet: %d", valor, Player[0].valorap);
			text_info->setText(text);
		}
		if (dataCard->Mano == &Player[1])
		{
			text_info = this->getDescendantT<TextField>("player_text_1", ep_ignore_error);
			valor = CheckValor(dataCard->Mano, dataCard->CurrentNum);
			safe_sprintf(text, "Mano: %d\nBet: %d", valor, Player[1].valorap);
			text_info->setText(text);
		}
		if (dataCard->Mano == &Player[2])
		{
			text_info = this->getDescendantT<TextField>("player_text_2", ep_ignore_error);
			valor = CheckValor(dataCard->Mano, dataCard->CurrentNum);
			safe_sprintf(text, "Mano: %d\nBet: %d", valor, Player[2].valorap);
			text_info->setText(text);
		}
		if (dataCard->Mano == &Player[3])
		{
			text_info = this->getDescendantT<TextField>("player_text_3", ep_ignore_error);
			valor = CheckValor(dataCard->Mano, dataCard->CurrentNum);
			safe_sprintf(text, "Mano: %d\nBet: %d", valor, Player[3].valorap);
			text_info->setText(text);
		}
		return;
	}

	void cardStopFlip(Event* event)
	{
		spSprite sprite;
		spSprite card;
		Vector2 srcPos;
		float angle;
		struct_datacards* dataCard;

		card = new Sprite();
		addChild(card);
		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		srcPos = sprite->getPosition();
		angle = sprite->getRotationDegrees();
		card->setPosition(srcPos);
		card->setRotationDegrees(angle);
		card->setScale(scaleFactor);
		card->setAnchor(0.5f, 0.5f);
		card->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		dataCard->card = card;
		card->setUserData(dataCard);
		if (dataCard->bFlip)
		{
			dataCard->bFlip = false;
			dataCard = (struct_datacards *)SendBuffer.Get(true);
		}
		cardShowText(dataCard);
		return;
	}

	void cardEndFlip(Event* event)
	{
		spSprite sprite;
		spTween tween;
		struct_datacards* dataCard;

		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		sprite->setScale(scaleFactor);
		sprite->setAnchor(0.5f, 0.5f);
		sprite->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		tween = sprite->addTween(Actor::TweenWidth(CardBaseSize.x / scaleFactor), 200, 1, false);
		tween->addDoneCallback(CLOSURE(this, &MainDeck::cardStopFlip));
		tween->detachWhenDone();
		return;

	}

	void cardStartFlip(Event* event)
	{
		spSprite sprite;
		spTween tween;
		struct_datacards* dataCard;

		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		sprite->setUserData(dataCard);
		tween = sprite->addTween(Actor::TweenWidth(0), 200, 1, false);
		tween->addDoneCallback(CLOSURE(this, &MainDeck::cardEndFlip));
		return;
	}

	void ButtonUp(Event* event)
	{
		spSprite button;

		if (event->currentTarget->getName().find("button") == std::string::npos)
			return;
//		if (event->target->getName() == "touchBlocker")
//			return;

		button = safeSpCast<Sprite>(event->currentTarget);
		button->setScale(0.9f * gameScale * buttonScale);
		button->addTween(Actor::TweenScale(gameScale * buttonScale), 250, 1, false);
		return;
	}

	void ButtonDown(Event* event)
	{
		spSprite button;

		if (event->currentTarget->getName().find("button") == std::string::npos)
			return;
//		if (event->target->getName() == "touchBlocker")
//			return;

		button = safeSpCast<Sprite>(event->currentTarget);
		button->setScale(gameScale * buttonScale);
		button->addTween(Actor::TweenScale(0.9f * gameScale * buttonScale), 250, 1, false);
		return;
	}

	void CheckCroupier(void)
	{
		spSprite	sprite;
		spSprite	card;
		spTween		tween;
		char text[255];

		if (!started)
			started = 1;

		DisableActionButtons();

		card = Mazo[Croupier.inuse[1]].card;
		Mazo[Croupier.inuse[1]].bFlip = true;
		SendBuffer.Add(&Mazo[Croupier.inuse[1]]);

		tween = card->addTween(TweenDummy(), 500);
		tween->addDoneCallback(CLOSURE(this, &MainDeck::cardStartFlip));

		Croupier.valor = CheckValor(&Croupier, Croupier.used);
		safe_sprintf(text, "CheckCroupier en < 17 Croupier.valor = %d", Croupier.valor);
		logs::messageln(text);

		while (Croupier.valor < 17)
		{
			Croupier.inuse[Croupier.used] = Mazo[crdsplyed].iValor;
			CardShow(&Croupier, Croupier.inuse[Croupier.used], Croupier.used, true);
			crdsplyed = crdsplyed + 1;
			Croupier.used = Croupier.used + 1;
			Croupier.valor = CheckValor(&Croupier, Croupier.used);
		}
		if (Croupier.valor == 17)
		{
			if (CheckSoftHand(&Croupier))
			{
				do
				{
					Croupier.inuse[Croupier.used] = Mazo[crdsplyed].iValor;
					CardShow(&Croupier, Croupier.inuse[Croupier.used], Croupier.used, true);
					crdsplyed = crdsplyed + 1;
					Croupier.used = Croupier.used + 1;
					Croupier.valor = CheckValor(&Croupier, Croupier.used);
				} while (Croupier.valor < 17);
			}
		}
		safe_sprintf(text, "CheckCroupier End Croupier.valor = %d", Croupier.valor);
		logs::messageln(text);
		CheckRules = true;
		return;
	}

	bool CheckAs(void)
	{
		if ((mazos > 0) && (Mazo[Player[mazoact].inuse[0]].pCard.x == ACE) && (Mazo[Player[mazoact].inuse[1]].pCard.x == ACE))
			return 1;
		else
			return 0;
	}

	bool CheckSame(void)
	{
		Point	temp;

		if (Player[mazoact].used == 2)
		{
			switch (Mazo[Player[mazoact].inuse[0]].pCard.x)
			{
				default:
					temp.x = Mazo[Player[mazoact].inuse[0]].pCard.x;
				break;
				case JACK:
				case QUEEN:
				case KING:
					temp.x = TEN;
				break;
			}
			switch (Mazo[Player[mazoact].inuse[1]].pCard.x)
			{
				default:
					temp.y = Mazo[Player[mazoact].inuse[1]].pCard.x;
				break;
				case JACK:
				case QUEEN:
				case KING:
					temp.y = TEN;
				break;
			}

			if (temp.x == temp.y)
				return 1;
			else
			{
				flow::show(new MessageBoxPrintf(mb_info, 2000, "NOT SAME CARDS", "Las Cartas no Corresponden para Abrir su mazo"), CLOSURE(this, &MainDeck::PostDialogFinish));
				return 0;
			}
		}
		else
			flow::show(new MessageBoxPrintf(mb_info, 2000, "YOU CAN'T SPLIT", "Solo puede abrir con 2 cartas en la Mano."), CLOSURE(this, &MainDeck::PostDialogFinish));
		return 0;
	}

	void SplitHand(Event* event)
	{
		int k;

		if (event->currentTarget->getName() != "button_split")
			return;
//		if (event->target->getName() == "touchBlocker")
//			return;

		if ((Player[0].valorap + Player[1].valorap + Player[2].valorap + Player[3].valorap + apinicial) > money)
			flow::show(new MessageBoxPrintf(mb_info, 2000, "NOT ENOUGH MONEY", "No tiene suficiente Dinero para Abir su Mazo"), CLOSURE(this, &MainDeck::PostDialogFinish));
		else if (mazos == 3)
			flow::show(new MessageBoxPrintf(mb_info, 2000, "YOU CAN'T SPLIT AGAIN", "No puede Abrir mas Manos. Ya tiene 3 abiertas"), CLOSURE(this, &MainDeck::PostDialogFinish));
		else if (CheckAs())
			flow::show(new MessageBoxPrintf(mb_info, 2000, "YOU CAN'T SPLIT AGAIN", "No puede Abrir con ASES. Ya tiene una mano de Ases abiertas"), CLOSURE(this, &MainDeck::PostDialogFinish));
		else if (CheckSame())
		{
			Player[mazoact].used = Player[mazoact].used - 1;
			Player[mazos + 1].inuse[Player[mazos + 1].used] = Player[mazoact].inuse[Player[mazoact].used];
			cardShowText((struct_datacards*)Mazo[Player[mazoact].inuse[Player[mazoact].used-1]].card->getUserData());

			mazos = mazos + 1;
			Player[mazos].valorap = apinicial;
			CardShow(&Player[mazos], Player[mazoact].inuse[Player[mazoact].used], Player[mazos].used, false);
			cardShowText((struct_datacards*)Mazo[Player[mazoact].inuse[Player[mazoact].used]].card->getUserData());
			Player[mazos].used = Player[mazos].used + 1;

			Player[mazoact].inuse[Player[mazoact].used] = Mazo[crdsplyed].iValor;
			CardShow(&Player[mazoact], Player[mazoact].inuse[Player[mazoact].used], Player[mazoact].used, true);
			Player[mazoact].used = Player[mazoact].used + 1;
			crdsplyed = crdsplyed + 1;

			Player[mazos].inuse[Player[mazos].used] = Mazo[crdsplyed].iValor;
			CardShow(&Player[mazos], Player[mazos].inuse[Player[mazos].used], Player[mazos].used, true);
			Player[mazos].used = Player[mazos].used + 1;
			crdsplyed = crdsplyed + 1;

			for (k = 0; k < mazos + 1; k++)
			{
				Player[k].valor = CheckValor(&Player[k], Player[k].used);
			}
		}
		return;
	}

	bool CheckDouble(void)
	{
		if (Player[mazoact].used == 2)
			return 1;
		else
			return 0;
	}

	void Double(Event* event)
	{
		int k;

		if (event->currentTarget->getName() != "button_double")
			return;
//		if (event->target->getName() == "touchBlocker")
//			return;

		if (!CheckDouble())
		{
			flow::show(new MessageBoxPrintf(mb_info, 2000, "YOU CAN'T DOUBLE", "Solo Puede Doblar con 2 cartas en la Mano."), CLOSURE(this, &MainDeck::PostDialogFinish));
			return;
		}
		if ((Player[0].valorap + Player[1].valorap + Player[2].valorap + Player[3].valorap + apinicial) > money)
		{
			flow::show(new MessageBoxPrintf(mb_info, 2000, "NOT ENOUGH MONEY", "No tiene suficiente Dinero para Doblar su Apuesta"), CLOSURE(this, &MainDeck::PostDialogFinish));
			return;
		}
		Player[mazoact].inuse[Player[mazoact].used] = Mazo[crdsplyed].iValor;
		CardShow(&Player[mazoact], Player[mazoact].inuse[Player[mazoact].used], Player[mazoact].used, true);
		crdsplyed = crdsplyed + 1;
		Player[mazoact].used = Player[mazoact].used + 1;
		Player[mazoact].valorap = Player[mazoact].valorap * 2;
		for (k = 0; k < mazos + 1; k++)
		{
			Player[k].valor = CheckValor(&Player[k], Player[k].used);
		}
		if ((mazos > 0) && (mazoact != mazos))
			mazoact = mazoact + 1;
		else
			CheckCroupier();
		return;
	}

	void StandGame(Event* event)
	{

		if (event->currentTarget->getName() != "button_stand")
			return;
//		if (event->target->getName() == "touchBlocker")
//			return;

		if ((mazos > 0) && (mazoact != mazos))
			mazoact = mazoact + 1;
		else
			CheckCroupier();
		return;
	}

	void HitGame(Event* event)
	{
		int k;
	
		if (event->currentTarget->getName() != "button_hit")
			return;
//		if (event->target->getName() == "touchBlocker")
//			return;

		if (Player[mazoact].used > 7)
		{
			flow::show(new MessageBoxPrintf(mb_info, 2000, "YOU CAN'T HIT AGAIN", "No puede sacar mas Cartas.\nYa tiene el Maximo de cartas permitidos en esa mano."), CLOSURE(this, &MainDeck::PostDialogFinish));
			return;
		}
		else
		{
			Player[mazoact].inuse[Player[mazoact].used] = Mazo[crdsplyed].iValor;
			CardShow(&Player[mazoact], Player[mazoact].inuse[Player[mazoact].used], Player[mazoact].used, true);
			Player[mazoact].used = Player[mazoact].used + 1;
			crdsplyed = crdsplyed + 1;
			for (k = 0; k < mazos + 1; k++)
			{
				Player[k].valor = CheckValor(&Player[k], Player[k].used);
			}
			if (Player[mazoact].valor > 21)
			{
				if (mazoact == mazos)
					CheckCroupier();
				else
					mazoact = mazoact + 1;
			}
		}
		return;
	}

	void ClearTable(void)
	{
		spSprite	card;
		spTween		tween;
		int i,k;

		for (k = 0; k < mazos + 1; k++)
		{
			for (i = 0; i < Player[k].used; i++)
			{
				card = Mazo[Player[k].inuse[i]].card;
				card->detach();
				Mazo[Player[k].inuse[i]].card = NULL;
			}
			player_text[k]->setText("");
		}
		for (i = 0; i < Croupier.used; i++)
		{
			card = Mazo[Croupier.inuse[i]].card;
			card->detach();
			Mazo[Croupier.inuse[i]].card = NULL;
		}
		croupier_text->setText("");
		return;
	}

	void ResetTableData(void)
	{
		seguro = 0;
		mazoact = 0;
		mazos = 0;
		apuesta = 0;
		started = 0;
		Croupier.valor = 0;
		Croupier.used = 0;
		Player[0].valor = 0;
		Player[1].valor = 0;
		Player[2].valor = 0;
		Player[3].valor = 0;
		Player[0].used = 0;
		Player[1].used = 0;
		Player[2].used = 0;
		Player[3].used = 0;

		return;
	}

	void PlayAgain(Event* event)
	{
		if (event->currentTarget->getName() == "btn_yes")
		{
			ClearTable();
			ResetTableData();
			EnableBetButtons();
		}
		return;
	}

	void BuyInsurance(Event* event)
	{
		char text[255];

		if (event->currentTarget->getName() == "btn_yes")
		{
			money = money - (int)(apinicial*0.5);
			seguro = 1;
			safe_sprintf(text, "Money: %d", money);
			money_text->setText(text);
		}
		return;
	}

	void GiveUp(Event* event)
	{
		char text[255];

		if (event->currentTarget->getName() == "btn_yes")
		{
			money = money - (int)(apinicial*0.5);
			safe_sprintf(text, "Money: %d", money);
			money_text->setText(text);
			DisableActionButtons();
			ClearTable();
			ResetTableData();
			EnableBetButtons();
		}
		return;
	}

	void CheckOfertas(void)
	{
		if (Croupier.valor == 11)
			flow::show(new MessageBoxPrintf(mb_yes_no, 0, "SAFETY BET", "Croupier tiene un As\nDesea Comprar un seguro por %d", (int)(apinicial*0.5)), CLOSURE(this, &MainDeck::BuyInsurance));
		if (Croupier.valor == 10)
			flow::show(new MessageBoxPrintf(mb_yes_no, 0, "GIVE UP", "Croupier Ofrece la oportunidad de rendirse\nDesea Rendirse por %d", (int)(apinicial*0.5)), CLOSURE(this, &MainDeck::GiveUp));
		return;
	}

	bool CheckSoftHand(struct_mano* Mano)
	{
		int i, valor, has;

		valor = 0;
		has = 0;

		for (i = 0; i < Mano->used; i++)
		{
			if (Mazo[Mano->inuse[i]].pCard.x == ACE)
			{
				valor = valor + 11;
				has = has + 1;
			}
			else if ((Mazo[Mano->inuse[i]].pCard.x == JACK) || (Mazo[Mano->inuse[i]].pCard.x == QUEEN) ||
				(Mazo[Mano->inuse[i]].pCard.x == KING))
				valor = valor + 10;
			else
				valor = valor + (Mazo[Mano->inuse[i]].pCard.x + 1);
		}
		if ((valor < 21) && (has))
			return true;
		return false;
	}

	int CheckValor(struct_mano* Mano, int num)
	{
		int i, valor, has;

		valor = 0;
		has = 0;

		for (i = 0; i < num; i++)
		{
			if (Mazo[Mano->inuse[i]].pCard.x == ACE)
			{
				valor = valor + 11;
				has = has + 1;
			}
			else if ((Mazo[Mano->inuse[i]].pCard.x == JACK) || (Mazo[Mano->inuse[i]].pCard.x == QUEEN) ||
				(Mazo[Mano->inuse[i]].pCard.x == KING))
				valor = valor + 10;
			else
				valor = valor + (Mazo[Mano->inuse[i]].pCard.x + 1);
		}
		if ((valor > 21) && (has))
		{
			for (i = 0; i < has; i++)
			{
				if (valor > 21)
					valor = valor - 10;
			}
		}
		return valor;
	}

	void GetNextCard(Event* event)
	{
		spSprite	sprite;
		struct_datacards* dataCard;

		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		dataCard->bFlip = false;
		dataCard = (struct_datacards *)SendBuffer.Get(true);
		return;
	}

	void SendCard(struct_datacards* dataCard)
	{
		spSprite	card;
		spTween		tween;

		if (dataCard->card == NULL)
		{
			card = new Sprite();
			addChild(card);
			card->setScale(scaleFactor);
			card->setAnchor(0.5f, 0.5f);
			card->setAnimFrame((_back), 1);

			if (crdsplyed < 250)
			{
				card->setX(CardDeck[1].Pos.x);
				card->setY(CardDeck[1].Pos.y);
			}
			else
			{
				if (CardDeck[1].card != NULL)
					CardDeck[1].card->detach();
				card->setX(CardDeck[0].Pos.x);
				card->setY(CardDeck[0].Pos.y);
			}

			if (crdsplyed < 200)
			{
				card->setX(CardDeck[2].Pos.x);
				card->setY(CardDeck[2].Pos.y);
			}
			else if (CardDeck[2].card != NULL)
				CardDeck[2].card->detach();

			if (crdsplyed < 150)
			{
				card->setX(CardDeck[3].Pos.x);
				card->setY(CardDeck[3].Pos.y);
			}
			else if (CardDeck[3].card != NULL)
				CardDeck[3].card->detach();

			if (crdsplyed < 100)
			{
				card->setX(CardDeck[4].Pos.x);
				card->setY(CardDeck[4].Pos.y);
			}
			else if (CardDeck[4].card != NULL)
				CardDeck[4].card->detach();

			if (crdsplyed < 50)
			{
				card->setX(CardDeck[5].Pos.x);
				card->setY(CardDeck[5].Pos.y);
			}
			else if (CardDeck[5].card != NULL)
				CardDeck[5].card->detach();

		}
		else
			card = dataCard->card;
		dataCard->bFlip = true;
		dataCard->card = card;
		card->setUserData(dataCard);

		if (!dataCard->angle)
			dataCard->angle = 180;
		card->addTween(Sprite::TweenRotationDegrees(dataCard->angle), 500, 1, false);
		tween = card->addTween(Sprite::TweenPosition(dataCard->EndPos), 500, 1, false);
		if (dataCard->show)
			tween->addDoneCallback(CLOSURE(this, &MainDeck::cardStartFlip));
		else
			tween->addDoneCallback(CLOSURE(this, &MainDeck::GetNextCard));
		return;
	}

	void EnableActionButtons(void)
	{
		spSprite button;

		button = this->getDescendantT<Sprite>("button_hit", ep_ignore_error);
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::HitGame));
		button->addTween(Sprite::TweenColor(Color(255, 255, 255, 255)), 300);

		button = this->getDescendantT<Sprite>("button_stand", ep_ignore_error);
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::StandGame));
		button->addTween(Sprite::TweenColor(Color(255, 255, 255, 255)), 300);

		button = this->getDescendantT<Sprite>("button_double", ep_ignore_error);
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::Double));
		button->addTween(Sprite::TweenColor(Color(255, 255, 255, 255)), 300);

		button = this->getDescendantT<Sprite>("button_split", ep_ignore_error);
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::SplitHand));
		button->addTween(Sprite::TweenColor(Color(255, 255, 255, 255)), 300);

		return;
	}
	
	void DisableActionButtons(void)
	{
		spSprite button;

		button = this->getDescendantT<Sprite>("button_hit", ep_ignore_error);
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->removeEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::HitGame));
		button->addTween(Sprite::TweenColor(Color(127, 127, 127, 255)), 300);

		button = this->getDescendantT<Sprite>("button_stand", ep_ignore_error);
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->removeEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::StandGame));
		button->addTween(Sprite::TweenColor(Color(127, 127, 127, 255)), 300);

		button = this->getDescendantT<Sprite>("button_double", ep_ignore_error);
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->removeEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::Double));
		button->addTween(Sprite::TweenColor(Color(127, 127, 127, 255)), 300);

		button = this->getDescendantT<Sprite>("button_split", ep_ignore_error);
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->removeEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::SplitHand));
		button->addTween(Sprite::TweenColor(Color(127, 127, 127, 255)), 300);

		return;
	}

	void EnableBetButtons(void)
	{
		spSprite button;

		button = this->getDescendantT<Sprite>("button_plus", ep_ignore_error);
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ChangeBet));
		button->addTween(Sprite::TweenColor(Color(255, 255, 255, 255)), 300);

		button = this->getDescendantT<Sprite>("button_minus", ep_ignore_error);
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ChangeBet));
		button->addTween(Sprite::TweenColor(Color(255, 255, 255, 255)), 300);

		button = this->getDescendantT<Sprite>("button_bet", ep_ignore_error);
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::CheckBet));
		button->addTween(Sprite::TweenColor(Color(255, 255, 255, 255)), 300);

		return;
	}

	void DisableBetButtons(void)
	{
		spSprite button;

		button = this->getDescendantT<Sprite>("button_plus", ep_ignore_error);
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ChangeBet));
		button->addTween(Sprite::TweenColor(Color(127, 127, 127, 255)), 300);

		button = this->getDescendantT<Sprite>("button_minus", ep_ignore_error);
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ChangeBet));
		button->addTween(Sprite::TweenColor(Color(127, 127, 127, 255)), 300);

		button = this->getDescendantT<Sprite>("button_bet", ep_ignore_error);
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonDown));
		button->removeEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::ButtonUp));
		button->removeEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::CheckBet));
		button->addTween(Sprite::TweenColor(Color(127, 127, 127, 255)), 300);

		return;
	}
	void CheckCardBuffer(void)
	{
		struct_datacards* dataCard;

		if (SendBuffer.isEmpty())
		{
			if (CheckOferta)
			{
				CheckOferta = false;
				CheckOfertas();
			}
			else if (CheckRules)
			{
				CheckRules = false;
				DisableActionButtons();
				CheckEndGame();
			}
			return;
		}
		else
		{
			dataCard = (struct_datacards *)SendBuffer.Get(false);
			if (dataCard->bFlip)
				return;
			else
				SendCard(dataCard);
		}

		return;
	}

	void CardShow(struct_mano* Mano, int valor, int num, bool show)
	{
		spSprite	card;
		spTween		tween;

		Mazo[valor].EndPos.x = Mano->Pos.x + num*Mano->Dist.x;
		Mazo[valor].EndPos.y = Mano->Pos.y + num*Mano->Dist.y;
		if ((Mano == &Croupier) && (num))
			Mazo[valor].EndPos.x += CardBaseSize.x;
		Mazo[valor].show = show;
		Mazo[valor].angle = Mano->angle;
		Mazo[valor].Mano = Mano;
		Mazo[valor].CurrentNum = num + 1;
		SendBuffer.Add(&Mazo[valor]);

		return;
	}

	void BeginGame(void)
	{

		if (crdsplyed > 282)
			CardsShf();

		ResetTableData();

		Player[0].inuse[Player[0].used] = Mazo[crdsplyed].iValor;
		CardShow(&Player[0], Player[0].inuse[Player[0].used], Player[0].used, true);
		crdsplyed = crdsplyed + 1;
		Player[0].used = Player[0].used + 1;
		
		Croupier.inuse[Croupier.used] = Mazo[crdsplyed].iValor;
		CardShow(&Croupier, Croupier.inuse[Croupier.used], Croupier.used, true);
		crdsplyed = crdsplyed + 1;
		Croupier.used = Croupier.used + 1;
		Croupier.valor = CheckValor(&Croupier, Croupier.used);

		Player[0].inuse[Player[0].used] = Mazo[crdsplyed].iValor;
		CardShow(&Player[0], Player[0].inuse[Player[0].used], Player[0].used, true);
		crdsplyed = crdsplyed + 1;
		Player[0].used = Player[0].used + 1;
		Player[0].valor = CheckValor(&Player[0], Player[0].used);

		Croupier.inuse[Croupier.used] = Mazo[crdsplyed].iValor;
		CardShow(&Croupier, Croupier.inuse[Croupier.used], Croupier.used, false);
		crdsplyed = crdsplyed + 1;
		Croupier.used = Croupier.used + 1;
	}

	void CheckBet(Event* event)
	{
		char text[255];

		if (apuesta)
			return;

		if (apinicial > money)
		{
			flow::show(new MessageBoxPrintf(0, 2000, "HIGH BET", "Apuesta muy alta. Usted no tiene el Dinero Suficiente"), CLOSURE(this, &MainDeck::PostDialogFinish));
			return;
		}
		else
		{
			Player[0].valorap = apinicial;
			Player[1].valorap = 0;
			Player[2].valorap = 0;
			Player[3].valorap = 0;
			safe_sprintf(text, "Bet: %d", apinicial);
			player_text[0]->setText(text);
			BeginGame();
			DisableBetButtons();
			EnableActionButtons();
			apuesta = 1;
			CheckOferta = true;
		}
		return;
	}

	void ChangeBet(Event* event)
	{
		spSprite button;
		spTextField BetText;
		spMessageBoxPrintf Message_Box;
		bool* bRiseBet;
		char text[255];

		button = safeSpCast<Sprite>(event->target);
		bRiseBet = (bool*)button->getUserData();
		BetText = getStage()->getDescendantT<TextField>("BetText", ep_ignore_error);
		button->setScale(gameScale * plusminusScale);
		button->addTween(Actor::TweenScale(0.5f * gameScale * plusminusScale), 250, 1, true);
		if (*bRiseBet)
		{
			if (apinicial >= money)
				flow::show(new MessageBoxPrintf(mb_info, 2000, "HIGH BET", "Apuesta muy alta. Usted no tiene el Dinero Suficiente"), CLOSURE(this, &MainDeck::PostDialogFinish));
			else
				apinicial += 50;
		}
		else
		{
			if (apinicial < 101)
				flow::show(new MessageBoxPrintf(mb_info, 2000, "LOW BET", "Apuesta muy baja.\n La puesta Minima es de 100"), CLOSURE(this, &MainDeck::PostDialogFinish));
			else
				apinicial -= 50;				
		}
		safe_sprintf(text, "%d", apinicial);
		BetText->setText(text);
		return;
	}

	void CheckEndGame(void) 
	{
		int k;
		char text[255];

		if ((Player[0].valor == 21) && (Player[0].used == 2) && (mazos == 0))
		{
			flow::show(new MessageBoxPrintf(mb_yes_no, 0, "BLACKJACK", "Usted ha Ganado Tiene un BlackJack\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
			money = money + (int)(Player[0].valorap * 1.5);
		}
		else if ((mazoact == mazos) && (started))
		{
			for (k = 0; k < mazos + 1; k++)
			{
				if (Player[k].valor > 21)
				{
					if (k == mazos)
						flow::show(new MessageBoxPrintf(mb_yes_no, 0, "PLAYER BUSTED", "Usted ha Perdido Tiene mas de 21\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
					money = money - Player[k].valorap;
				}
				else if (Croupier.valor > 21)
				{
					if (k == mazos)
						flow::show(new MessageBoxPrintf(mb_yes_no, 0, "DEALER BUSTED", "Usted ha Ganado, Croupier tiene mas de 21\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
					money = money + Player[k].valorap;
				}
				else if (Player[k].valor > Croupier.valor)
				{
					if (k == mazos)
						flow::show(new MessageBoxPrintf(mb_yes_no, 0, "PLAYER WINS", "Usted ha Ganado, tiene mas que el Croupier\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
					money = money + Player[k].valorap;
				}
				else if (Player[k].valor == Croupier.valor)
				{
					if ((Croupier.valor == 21) && (Croupier.used == 2))
					{
						if (!seguro)
						{
							if (k == mazos)
								flow::show(new MessageBoxPrintf(mb_yes_no, 0, "BLACKJACK WINS", "Croupier ha Ganado con un BlackJack\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
							money = money - Player[k].valorap;
						}
						else if ((seguro) && (k == mazos))
							flow::show(new MessageBoxPrintf(mb_yes_no, 0, "BLACKJACK WINS", "Croupier Tiene un BlackJack, Pero su seguro Cubre la Perdida\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
					}
					else if (k == mazos)
						flow::show(new MessageBoxPrintf(mb_yes_no, 0, "DRAW", "Empate. Usted como el Croupier tiene el mismo valor\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
				}
				else if (Player[k].valor < Croupier.valor)
				{
					if ((Croupier.valor == 21) && (Croupier.used == 2))
					{
						if (!seguro)
						{
							if (k == mazos)
								flow::show(new MessageBoxPrintf(mb_yes_no, 0, "BLACKJACK WINS", "Croupier ha Ganado con un BlackJack\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
							money = money - Player[k].valorap;
						}
						else if ((seguro) && (k == mazos))
							flow::show(new MessageBoxPrintf(mb_yes_no, 0, "BLACKJACK WINS", "Croupier Tiene un BlackJack, Pero su seguro Cubre la Perdida\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
					}
					else if (k == mazos)
						flow::show(new MessageBoxPrintf(mb_yes_no, 0, "DEALER WINS", "Croupier ha Ganado, tiene mas que Usted\nDesea Continuar"), CLOSURE(this, &MainDeck::PlayAgain));
					money = money - Player[k].valorap;
				}
			}
		}
		safe_sprintf(text, "Money: %d", money);
		money_text->setText(text);
		return;
	}
	
	void PostDialogFinish(Event* event)
	{
		if (event->currentTarget->getName() == "btn_ok_default")
		{
			return;
		}
		logs::messageln("dialog closed");
		return;
	}

};

typedef oxygine::intrusive_ptr<MainDeck> spMainDeck;
spMainDeck MainActor;

MessageBoxPrintf::MessageBoxPrintf(int Type, timeMS duration, std::string szCaption, std::string szFormat, ...)
{
	spSprite	button;
	spTextField	button_text;
	spTween tween;
	TextStyle style;
	std::size_t StringPos,PreviousPos;
	std::string SubString;
	unsigned int len;
	int width, height, addheight;

	setName("MessageBoxPrintf");

	_dialog = true;
	_passBlockedTouch = false;

	va_list pArgList, pArgList_copy;
	va_start(pArgList, szFormat);
	va_copy(pArgList_copy, pArgList);

	std::string szBuffer(vsnprintf(nullptr, 0, szFormat.c_str(), pArgList) + 1, ' ');
	vsnprintf(&szBuffer.front(), vsnprintf(nullptr, 0, szFormat.c_str(), pArgList) + 1, szFormat.c_str(), pArgList_copy);

	va_end(pArgList_copy);
	va_end(pArgList);

	len = 0;
	width = 0;
	height = 60;
	addheight = 0;

	PreviousPos = 0;
	SubString = szBuffer;
	do
	{
		height += 40;
		StringPos = SubString.find("\n");
		if (StringPos != std::string::npos)
		{
			PreviousPos = StringPos - PreviousPos;
			SubString = SubString.substr(StringPos+1);
			if (len < PreviousPos)
				len = PreviousPos;
		}
		else if (len < SubString.length())
			len = SubString.length();
	} while (StringPos != std::string::npos);


	if (Type)
	{
		height += 100;
		addheight = 100;
	}
	width = len * 18;

	spBox9Sprite view = new Box9Sprite();
	_view = view;
	view->setResAnim(gameResources.getResAnim("msgbox"));
	view->setAnchor(0.5f, 0.5f);
	view->setPosition(_holder->getSize() / 2);
	view->setSize((float)width, (float)height);
	view->setVerticalMode(Box9Sprite::STRETCHING);
	view->setHorizontalMode(Box9Sprite::STRETCHING);
	view->setAlpha(200);
	view->attachTo(_holder);

	spTextField TextMessage = new TextField();
	TextMessage->attachTo(_view);
	TextMessage->setX(_view->getWidth() / 2);
	TextMessage->setY(_view->getHeight() / 2 - addheight);
	TextMessage->setPosition(_view->getSize() / 2);
	style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
	TextMessage->setStyle(style);
	TextMessage->setText(szBuffer);
	if (duration)
	{
		tween = _view->addTween(Sprite::TweenAlpha(0), 300, 1, false, duration);
		tween->setName("btn_ok_default");
		tween->addDoneCallback(CLOSURE(this, &MessageBoxPrintf::RemoveMessageBox));
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			button = new Sprite();
			if (i)
				button->setName("btn_no");
			else
				button->setName("btn_yes");
			button->setResAnim(gameResources.getResAnim("button"));
			button->setScale(0.75f);
			button->setAnchor(0.5f, 0.5f);
			button->setX(_view->getWidth() / 4 + i*_view->getWidth() / 2);
			button->setY(_view->getHeight() - addheight / 2);
			button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MessageBoxPrintf::RemoveMessageBox));
			button->attachTo(view);

			button_text = new TextField();
			if (i)
				button_text->setText("NO");
			else
				button_text->setText("SI");
			button_text->attachTo(button);
			button_text->setPosition(button->getSize() / 2);
			style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
			button_text->setStyle(style);
		}
	}
	addEventListener(EVENT_PRE_SHOWING, CLOSURE(this, &MessageBoxPrintf::PreShowMessageBox));
	addEventListener(EVENT_POST_SHOWING, CLOSURE(this, &MessageBoxPrintf::PostShowMessageBox));
	addEventListener(EVENT_BACK, CLOSURE(this, &MessageBoxPrintf::RemoveMessageBox));
}

void MessageBoxPrintf::PreShowMessageBox(Event* event)
{
	spBox9Sprite view;
	view = safeSpCast<Box9Sprite>(_view);
	view->setColor(Color(0, 80, 0, 255));
	return;
}

void MessageBoxPrintf::PostShowMessageBox(Event* event)
{
	spBox9Sprite view;
	view = safeSpCast<Box9Sprite>(_view);
	view->addTween(Sprite::TweenColor(Color::White), 300);
	return;
}

void MessageBoxPrintf::RemoveMessageBox(Event* event)
{
	finish(event);
	return;
}

class MyScene : public flow::Scene
{
public:
	MyScene()
	{
		setName("MyScene");

		spBox9Sprite view = new Box9Sprite;
		_view = view;
		view->setSize(_holder->getSize());
		view->attachTo(_holder);
		view->setColor(Color(0, 0, 0, 0));

		gameResources.loadXML("res.xml");
		MainActor = new MainDeck;
		view->addChild(MainActor);

	}

	spActor _view;
};


void blackjack_preinit() {}

//called from main.cpp
void blackjack_init()
{
	gameScale = getStage()->getHeight() / 1080.0f;
	//initialize oxygine_flow
	flow::init();

	//create scene and display it
	flow::show(new MyScene, [](Event * event)
	{
		logs::messageln("scene closed");
	});

}

//called each frame from main.cpp
void blackjack_update()
{
	//update oxygine-flow each frame
	flow::update();
	MainActor->CheckCardBuffer();
}

void blackjack_destroy()
{
	//free oxygine-flow
	flow::free();
	gameResources.free();
}