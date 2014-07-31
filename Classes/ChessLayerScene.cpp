#include "ChessLayerScene.h"
#include "AppMacros.h"
#include "SimpleAudioEngine.h"
#include "EngineUCCI.h"
#include "pthread.h"
USING_NS_CC;


static pthread_t npid;
static pthread_mutex_t mymutex=PTHREAD_MUTEX_INITIALIZER;

#define MAN_OLDER 1
#define TURN()      whoTurn += PP256; PP256 = -PP256; iSelected = 0
#define IS_ENEMY(cell) (abs((cell)-whoTurn) > 8)

static const short ccKnightDelta[4][2] = {{-33,-31},{-18,14},{-14,18},{31,33}};
static const short ccKingDelta[4] = {-16,-1,1,16};
static const short ccMinisterDelta[4] = {-34,-30,30,34};
static const short ccGuardDelta[4] = {-17,-15,15,17};
static const short ccSoldierDelta[3] = {-1,-16,1};
static const short ccKingPalace[18] = {
    0x36,0x37,0x38,0x46,0x47,0x48,0x56,0x57,0x58,
    0xc6,0xc7,0xc8,0xb6,0xb7,0xb8,0xa6,0xa7,0xa8};
static int PP256 = 256+8;

char ChessLayer::ccInBoard[256] = 
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char ccInPalace[256] = 
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
const static char* pieceFiles[]=
{
    "RK.GIF",
    "RA.GIF",
    "RB.GIF",
    "RN.GIF",
    "RR.GIF",
    "RC.GIF",
    "RP.GIF",
    "BK.GIF",
    "BA.GIF",
    "BB.GIF",
    "BN.GIF",
    "BR.GIF",
    "BC.GIF",
    "BP.GIF"
};


CCScene* ChessLayer::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    ChessLayer *layer = ChessLayer::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool ChessLayer::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(ChessLayer::menuCloseCallback));

    pCloseItem->setPosition(ccp(origin.x + visibleSize.width - pCloseItem->getContentSize().width/2 ,
                                origin.y + pCloseItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu, 1);



    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("chess.plist");

    CCSprite* pBoard = CCSprite::createWithSpriteFrameName("WOOD.GIF");
    CCSprite* pMan = CCSprite::createWithSpriteFrameName("BK.GIF");

    m_scale = visibleSize.width/pBoard->getContentSize().width;
    pBoard->setScale(m_scale);

    cellSize = pBoard->getContentSize();
    cellSize.height = cellSize.width;

    cellSize = cellSize*(57.f/521)*m_scale;

    pBoard->setPosition(ccp(visibleSize.width/2,visibleSize.height/2)+origin);

    originBoard = pBoard->getPosition();
    originBoard.x -= cellSize.width*4;
    originBoard.y += cellSize.height*4.5;

    this->addChild(pBoard,0);
    initBoard();
    this->setTouchEnabled(true);
    this->schedule(schedule_selector(ChessLayer::updateBoard),0.05);
    InitEngine();

    return true;
}


void ChessLayer::menuCloseCallback(CCObject* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	CCMessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
#else
    CCDirector::sharedDirector()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
#endif
}

void ChessLayer::registerWithTouchDispatcher()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this,0,true);
}

void ChessLayer::ccTouchEnded( cocos2d::CCTouch* touch, cocos2d::CCEvent* event )
{
    CCPoint pt = touch->getLocation() - originBoard;
    int w = pt.x / cellSize.width + 0.5;
    int h = pt.y / cellSize.height - 0.5;

    int index = (3-h)*16 + w + 3;
    if (index < 0 || index > 256)
    {
        return ;
    }

    short dstCell = chessBoard[index];
    
    if (iSelected)      //有棋子选中
    {
        short srcCell = chessBoard[iSelected];
        bool bWalk = CanWalk(iSelected,index); //能不能走

        if (dstCell == 0)       //目标空
        {
            if (bWalk)          //移动
            {
                move(iSelected,index);
            }
        } 
        else
        {
            if (IS_ENEMY(dstCell))      //目标是对方棋子
            {
                if (bWalk)
                {
                    move(iSelected,index);
                }
                
            }
            else
            {
                this->getChildByTag(index)->setScale(m_scale*1.2);
                if (index != iSelected)
                {
                    this->getChildByTag(iSelected)->setScale(m_scale);
                }
                iSelected = index;
                CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("CLICK.WAV");
            }
            
        }
        
        
    }
    else        //没有棋子选中
    {
        if (dstCell && !IS_ENEMY(dstCell))
        {
            CCNode* pNode = this->getChildByTag(index);
            pNode->setScale(m_scale*1.2);
            iSelected = index;
            CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("CLICK.WAV");
        }
        
    }

}

cocos2d::CCPoint ChessLayer::POSITION( int index )
{
    int h = index / 16 - 3;
    int w = index % 16 - 3;

    CCPoint pt = originBoard;
    pt.x += w*cellSize.width;
    pt.y -= h*cellSize.height;

    return pt;
    
}

void ChessLayer::initBoard()
{
    chessBoard[0x33] = Black_Chariot;
    chessBoard[0x3b] = Black_Chariot;
    chessBoard[0x34] = Black_Knight;
    chessBoard[0x3a] = Black_Knight;
    chessBoard[0x35] = Black_Minister;
    chessBoard[0x39] = Black_Minister;
    chessBoard[0x36] = Black_Guard;
    chessBoard[0x38] = Black_Guard;
    chessBoard[0x37] = Black_General;

    chessBoard[0x54] = Black_Cannon;
    chessBoard[0x5a] = Black_Cannon;
    chessBoard[0x63] = Black_Solider;
    chessBoard[0x65] = Black_Solider;
    chessBoard[0x67] = Black_Solider;
    chessBoard[0x69] = Black_Solider;
    chessBoard[0x6b] = Black_Solider;

    chessBoard[0xc3] = Red_Chariot;
    chessBoard[0xcb] = Red_Chariot;
    chessBoard[0xc4] = Red_Knight;
    chessBoard[0xca] = Red_Knight;
    chessBoard[0xc5] = Red_Minister;
    chessBoard[0xc9] = Red_Minister;
    chessBoard[0xc6] = Red_Guard;
    chessBoard[0xc8] = Red_Guard;
    chessBoard[0xc7] = Red_General;
 
    chessBoard[0xa4] = Red_Cannon;
    chessBoard[0xaa] = Red_Cannon;
    chessBoard[0x93] = Red_Solider;
    chessBoard[0x95] = Red_Solider;
    chessBoard[0x97] = Red_Solider;
    chessBoard[0x99] = Red_Solider;
    chessBoard[0x9b] = Red_Solider;

    for (int i = 0; i < 256; i++)
    {
        byte iPiece = chessBoard[i];
        if (iPiece > 0)
        {
            CCSprite* chessMan = CCSprite::createWithSpriteFrameName(pieceFiles[iPiece-1]);
            chessMan->setPosition(POSITION(i));
            chessMan->setScale(m_scale);
            this->addChild(chessMan,MAN_OLDER,i);

        }
        
    }
    
}

ChessLayer::ChessLayer()
{
    memset(chessBoard,0,sizeof(chessBoard));
    iSelected = 0;
    whoTurn = Red_General;
    shareMove = 0;
}

bool ChessLayer::ccTouchBegan( CCTouch *pTouch, CCEvent *pEvent )
{
    return true;
}

bool ChessLayer::CanWalk( int src,int dst )
{
    short srcCell = chessBoard[src];
    bool bWalk = false;

    if (!ccInBoard[src] || !ccInBoard[dst] || src == dst || !IS_ENEMY(dst))
    {
        return false;
    }
    

    switch (srcCell)
    {
    case Red_Knight:
    case Black_Knight:
        {
            for (int i = 0; i < 4; i++)
            {
                int leg = chessBoard[src+ccKingDelta[i]];
                if (leg == 0)
                    for (int j = 0; j < 2; j++)
                    {
                        int move = dst - src;
                        if (ccKnightDelta[i][j] == move)
                        {
                            bWalk = true;
                            break;
                        }
                    }

            }

        }
        break;
    case Red_Cannon:
    case Black_Cannon:
        {
            int w1 = src%16;
            int w2 = dst%16;
            int h1 = src/16;
            int h2 = dst/16;
            bool bHave = chessBoard[dst] > 0;
            int count = 0;
            if (h1 == h2)
            {
                bWalk = !bHave;
                int i1 = (w2-w1)/abs(w2-w1);
                for (int i = w1+i1; i != w2; i += i1)
                {
                    if(chessBoard[h1*16+i])
                    {
                        if (bHave) count++;
                        else
                        {
                            bWalk = false;
                            break;
                        }
                    }
                }
                if (bHave && count == 1) 
                    bWalk = true;
            }
            if (w1 == w2)
            {
                bWalk = !bHave;
                int i1 = (h2-h1)/abs(h2-h1);
                for (int i = h1+i1; i != h2; i += i1)
                {
                    if(chessBoard[i*16+w1])
                    {
                        if (bHave) count++;
                        else
                        {
                            bWalk = false;
                            break;
                        }
                    }
                }
                if (bHave && count == 1) 
                    bWalk = true;
            }
            
        }
        break;
    case Red_Chariot:
    case Black_Chariot:
        {
            int w1 = src%16;
            int w2 = dst%16;
            int h1 = src/16;
            int h2 = dst/16;
            if (h1 == h2)
            {
                bWalk = true;
                int i1 = (w2-w1)/abs(w2-w1);
                for (int i = w1+i1; i != w2; i += i1)
                {
                    if(chessBoard[h1*16+i])
                    {
                        bWalk = false;
                        break;
                    }
                }
            }
            if (w1 == w2)
            {
                bWalk = true;
                int i1 = (h2-h1)/abs(h2-h1);
                for (int i = h1+i1; i != h2; i += i1)
                {
                    if(chessBoard[i*16+w1])
                    {
                        bWalk = false;
                        break;
                    }
                }
            }
        }
        break;
    case Red_Minister:
    case Black_Minister:
        {
            bWalk = false;
            int move = dst -src;
            bool bRed = (srcCell == Red_Minister);
            bool bRiver = (bRed && dst <= 0x7b) || (!bRed && dst >= 0x83);
            for (int i = 0; i < sizeof(ccMinisterDelta); i++)
            {
                if (move == ccMinisterDelta[i] && chessBoard[src + ccGuardDelta[i]] == 0 && !bRiver)
                {
                    bWalk = true;
                    break;
                }
            }
            
            break;
        }
    case Red_General:
    case Black_General:
        {
            bWalk = false;
            int move = dst - src;
            for (int i = 0; i < sizeof(ccKingDelta); i++)
            {
                if (move == ccKingDelta[i] && ccInPalace[dst])
                {
                   bWalk = true;
                   break;
                }
            }
            break;
        }
    case Red_Guard:
    case Black_Guard:
        {
            bWalk = false;
            int move = dst - src;
            for (int i = 0; i < sizeof(ccGuardDelta); i++)
            {
                if (move == ccGuardDelta[i] && ccInPalace[dst])
                {
                    bWalk = true;
                    break;
                }
            }
            break;
        }
    case Red_Solider:
    case Black_Solider:
        {
            int move = dst - src;
            bool bRed = (srcCell == Red_Solider);
            int d = bRed ? 1:-1;
            bool bRiver = (bRed && src <= 0x7b) || (!bRed && src >= 0x83);
            if (bRiver)
            {
                bWalk = false;
                
                for (int i = 0; i < sizeof(ccSoldierDelta); i++)
                {
                    if (move == d*ccKingDelta[i])
                    {
                        bWalk = true;
                        break;
                    }
                }
            }
            else
            {
                if (move == d*-16)
                {
                    bWalk = true;
                    break;
                }
            }
            break;
        }
    default:
        bWalk = true;
    }

    return bWalk;
}


void* threadRead(void* p)
{
    ChessLayer* layer = (ChessLayer*) p;
    layer->WorkRead();
    return NULL;
}


void ChessLayer::InitEngine()
{


//    std::string fullfile = CCFileUtils::sharedFileUtils()->fullPathForFilename("ELEEYE");
//    g_engine.open(fullfile.c_str());

    CCLog("init");

    pthread_create(&npid,NULL,threadRead,this);

    g_engine.write("ucci\n");

    g_engine.write("setoption usebook false\n");

  //  g_engine.flush("ucciok\n");
}


 char ccFEN[] = "*kabnrcp*KABNRCP";
std::string ChessLayer::fen()
{
    char ccstr[1024] = "";
    for (int i = 0; i < 10; i++)
    {
        char line[500] = "";
        int count = 0;
        for (int j = 0; j < 9; j++)
        {
            int h = i+3;
            int w = j+3;
            short man = chessBoard[h*16+w];
            if(man == 0)
                count++;
            else
            {
                int index = man;
                if (man > 256)
                {
                    index = index - Black_General+1;
                }
                else
                    index += 8;
                char c = ccFEN[index];
                if (count)
                {
                    strcat(line,CCString::createWithFormat("%d",count)->getCString());
                    count = 0;
                }
                strcat(line,CCString::createWithFormat("%c",c)->getCString());
            }
        }
       if(count)
       {
           strcat(line,CCString::createWithFormat("%d",count)->getCString());
           count = 0;
       }
        strcat(ccstr,line);
        strcat(ccstr,"/");
    }
    
    ccstr[strlen(ccstr)-1] = 0;
    return ccstr;
}

void ChessLayer::move( int src,int dst )
{
    bool bCapture = chessBoard[dst];
    if(bCapture)
        removeChildByTag(dst);
    CCNode *pNode = getChildByTag(src);
//    pNode->setPosition(POSITION(dst));
    chessBoard[dst] = chessBoard[src];
    chessBoard[src] = 0;
    pNode->setScale(m_scale);
    pNode->setTag(dst);
    TURN();

    pNode->runAction(CCSequence::create(CCMoveTo::create(0.1,POSITION(dst)),CCCallFunc::create(this,callfunc_selector(ChessLayer::callbackMove)),NULL));

    CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(check()?"CHECK2.WAV":(bCapture?"CAPTURE.WAV":"MOVE2.WAV"));

}

std::string cm(int src,int dst)
{
    int w1,w2,h1,h2;
    w1 = src%16;
    h1 = src/16;
    w1 = src%16;
    h1 = src/16;

    w2 = dst%16;
    h2 = dst/16;
    w2 = dst%16;
    h2 = dst/16;

    char s = w1-3 + 'a';
   
    char d = 9-(h1-3 + '0');

    std::string ss;
    ss += s;ss += d;

    s = w2 - 3 + 'a';
    d  = 9-(h2 -3 + '0');

    ss += s; ss += d;

    return ss;
}

unsigned short mc(const char* move)
{
    byte w1 = move[0]-'a'+3;
    byte h1 = 9 - (move[1]-'0')+3;
    byte w2 = move[2]-'a'+3;
    byte h2 = 9 - (move[3]-'0')+3;

    byte s = h1*16+w1;
    byte d = h2*16+w2;

    return s*256+d;

}

void ChessLayer::manualMove( int src,int dst )
{
    move(src,dst);
    CCString *send = CCString::createWithFormat("position fen %s b - - 0 1\n",fen().c_str()) ;
    g_engine.write(send->getCString());
    g_engine.write("go depth 10\n");

    char outline[4096] = "";
    do 
    {
        memset(outline,0,4096);
        g_engine.getLine(outline);
    } while (strncmp(outline,"bestmove ",9) != 0);

    std::string line = outline;
    
    unsigned short m = mc(line.substr(9,4).c_str());

    move(m >> 8,(byte)m);
}

void ChessLayer::callbackMove()
{
    if (whoTurn > PP256)
    {
        CCString *send = CCString::createWithFormat("position fen %s b - - 0 1\n",fen().c_str()) ;
        g_engine.write(send->getCString());
        g_engine.write("go depth 2\n");

//         if (npid == 0)
//         {
//             char line[4096];
//             do
//             {
//                 memset(line,0,4096);
//                 g_engine.getLine(line);
//             }while(strncmp(line,"bestmove ",9) != 0);
//             std::string sline = line;
//             unsigned short m = mc(sline.substr(9,4).c_str());
// 
//             move(m >> 8,(byte)m);
//         }

    }

}

void ChessLayer::WorkRead()
{
    char lineout[4096];
    while(1)
    {
        memset(lineout,0,4096);
        g_engine.getLine(lineout);

        if (strncmp(lineout,"bye",3) == 0)
        {
            CCLog("thread exit");
            break;
        }
        if (strncmp(lineout,"bestmove ",9) == 0)
        {
            std::string line = lineout;
            unsigned short m = mc(line.substr(9,4).c_str());

            pthread_mutex_lock(&mymutex);
            shareMove = m;
            pthread_mutex_unlock(&mymutex);
        }
        
    }

}

ChessLayer::~ChessLayer()
{
    g_engine.exit();
}

void ChessLayer::updateBoard(float)
{
    pthread_mutex_lock(&mymutex);
    if (shareMove > 0)
    {
        move(shareMove >> 8,(byte)shareMove);
        shareMove = 0;
    }
    pthread_mutex_unlock(&mymutex);

}

bool ChessLayer::check()
{
    int General = (sdPlayer() == RED) ? Red_General:Black_General;
    int dst;
    for (int i = 0; i < 256; i++)
    {
        if(chessBoard[i] == General)
        {
            dst = i;
            break;
        }
    }

    for (int i = 0; i < 256; i++)
    {
        if (chessBoard[i]&&IS_ENEMY(chessBoard[i]) && CanWalk(i,dst))
        {
            CCLog("check");
            return true;
        }
        
    }
    
    return false;

}

int ChessLayer::sdPlayer()
{
    return whoTurn > 256 ? BLACK:RED;
}
