#ifndef __ChessLayer_SCENE_H__
#define __ChessLayer_SCENE_H__

#include "cocos2d.h"


#ifndef byte
typedef  unsigned char byte;
#endif

using namespace cocos2d;

class ChessLayer : public cocos2d::CCLayer
{
public:
    ChessLayer();
    ~ChessLayer();
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);
    
    // implement the "static node()" method manually
    CREATE_FUNC(ChessLayer);


    void registerWithTouchDispatcher();
    void ccTouchEnded(cocos2d::CCTouch* touche, cocos2d::CCEvent* event);
    bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);

    void callbackMove();
    void WorkRead();
    enum
    {
        Red_General = 1,
        Red_Guard,
        Red_Minister,
        Red_Knight,
        Red_Chariot,
        Red_Cannon,
        Red_Solider,
        Black_General = 8+256,
        Black_Guard,
        Black_Minister,
        Black_Knight,
        Black_Chariot,
        Black_Cannon,
        Black_Solider,
    };

    enum {RED,BLACK};



protected:
    void initBoard();
    CCPoint POSITION(int index);
    bool CanWalk(int src,int dst);
    void InitEngine();

    std::string fen();

    void move(int src,int dst);
    void manualMove(int src,int dst);

    void updateBoard(float);

    bool check();

    int sdPlayer();

protected:
    short chessBoard[256];
    static char ccInBoard[256];
    CCPoint originBoard;
    CCSize cellSize;
    float m_scale;
    int iSelected;

    int whoTurn;

    unsigned short m_lastMove;

    unsigned short shareMove;
};

#endif // __ChessLayer_SCENE_H__
