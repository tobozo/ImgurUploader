/*

  ESP32 AmigaBoingBall - A port of the famous Amiga Boing Ball Demo
  ported from https://github.com/niklasekstrom/boing_ball_python/
  Source: https://github.com/tobozo/ESP32-AmigaBoingBall

  MIT License

  Copyright (c) 2019 tobozo

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  -----------------------------------------------------------------------------

*/

#define tft M5.Lcd

TFT_eSprite ball = TFT_eSprite(&tft);
TFT_eSprite shadow = TFT_eSprite(&tft);
TFT_eSprite grid = TFT_eSprite(&tft);

static bool buzz_wall = false;
static bool buzz_floor = false;


struct AmigaBallConfig {
  long Framelength = 20;
  byte Wires = 7; // 0 = no wireframes
  uint16_t BGColor = tft.color565(0xa9, 0xa9, 0xa9);
  uint16_t GridColor =  tft.color565(0xac, 0x00, 0xac);
  uint16_t ShadowColor = tft.color565(0x66, 0x66, 0x66);
  uint16_t YPos = 0;
  uint16_t XPos = 0;
  uint16_t Width = tft.width();
  uint16_t Height = tft.height();
  uint16_t ScaleRatio = 5; // ball size will have this/nth of the window Height, bigger value means smaller ball
} amigaBallConfig;


class AmigaRulez {
  public:

    struct Points {
      float x = 0.00;
      float y = 0.00;
    };

    Points points[10][10];

    float deg2rad   = PI/180.0;
    float phase8Rad = PI/8.0; // 22.5 deg
    float phase4Rad = PI/4.0; // 45 deg
    float phase2Rad = PI/2.0; // 90 deg
    float twopi     = PI*2;
    float Phase     = 0.0;
    float velocityX = 2.1;
    float velocityY = 0.07;
    float angleY    = 0.0;
    
    float PhaseVelocity;
    float perspective[4];
    float XtoYratio;
    float YtoXratio;
    float TiltRad;

    bool AnimationDone;
    bool isMovingRight;
    bool isMovingUp = false;
    
    byte Wires;
    byte bytecounter = 0;

    int BounceMargin;

    long Framelength;
    long startedTick = millis();
    long lastTick    = millis();
    long processTicks = 0;

    float variableScale = Scale;
    float oldScale = Scale;
    float ScaleAmplitude = 8;
    float MaxScaleAmplitude;
    float AmplitudeFactor = 4;
    float TiltDeg = 17; // 17 degrees tilting to the right
    float LeftBoundary;
    float RightBoundary;
    float ShadowYPos = 0.00;
    float XPos;
    float YPos;
    float Width;
    float Height;
    
    float VCentering;
    float Scale;
    float YPosAmplitude;
    uint16_t ScaleRatio;
    uint16_t BGColor;
    uint16_t GridColor;
    uint16_t ShadowColor;
    float lastPositionX;
    float lastPositionY;
    float positionX;
    float positionY;

    int spriteWidth;
    int spriteHeight;
    int spriteCenterX;
    int spriteCenterY;

    void init( AmigaBallConfig config = amigaBallConfig ) {
      BGColor     = config.BGColor;
      GridColor   = config.GridColor;
      Framelength = config.Framelength;//33; // millis
      ScaleRatio  = config.ScaleRatio;
      ShadowColor = config.ShadowColor;
      XPos   = config.XPos;
      YPos   = config.YPos;
      Width  = config.Width;
      Height = config.Height;
      Wires  = config.Wires;

      setupValues();
      
      tft.fillRect(XPos, YPos, Width, Height, BGColor);

      if( Wires > 0 ) {


        shadow.createSprite( spriteWidth / 2, spriteHeight / 8 );
        shadow.fillSprite( BGColor );
        shadow.fillEllipse( shadow.width()/2, shadow.height()/2, shadow.width()/2-4, shadow.height()/2-2, ShadowColor );
        //shadow.pushSprite( positionX-spriteCenterX + shadow.width()/2, ShadowYPos+shadow.height() );

        
        grid.createSprite( Width/2, Height/2 ); // don't overflow heap
        
        grid.fillSprite(BGColor);
        drawGrid(grid, XPos, YPos, Width, Height, grid.width(), grid.height());
        grid.pushSprite( XPos, YPos, BGColor );
  
        grid.fillSprite(BGColor);
        drawGrid(grid, XPos+Width/2, YPos, Width, Height, grid.width(), grid.height());
        grid.pushSprite( XPos+Width/2, YPos, BGColor );
  
        grid.fillSprite(BGColor);
        drawGrid(grid, XPos, YPos+Height/2, Width, Height, grid.width(), grid.height());
        grid.pushSprite( XPos, YPos+Height/2, BGColor );
  
        grid.fillSprite(BGColor);
        drawGrid(grid, XPos+Width/2, YPos+Height/2, Width, Height, grid.width(), grid.height());
        grid.pushSprite( XPos+Width/2, YPos+Height/2, BGColor );
  
        grid.deleteSprite();

        
      }

      ball.createSprite(spriteWidth, spriteHeight);
      ball.fillSprite(BGColor); // Note: Sprite is filled with black when created

    }


    void setupValues() {
      Scale = Height/ScaleRatio;// 
      ScaleAmplitude = Scale/ AmplitudeFactor; // ball diameter will vary on this
      MaxScaleAmplitude = Scale + ScaleAmplitude;

      spriteWidth  = (MaxScaleAmplitude + AmplitudeFactor )*2;
      spriteHeight = (MaxScaleAmplitude + AmplitudeFactor )*2;

      spriteCenterX = spriteWidth/2 + spriteWidth%2;
      spriteCenterY = spriteHeight/2 - spriteHeight%2;
     
      YPosAmplitude = (Height-(Scale+ScaleAmplitude))/2; // ball will bounce on this span pixels
      VCentering = YPos + (Height-1) - (MaxScaleAmplitude + AmplitudeFactor);// -(YPosAmplitude/2 + Scale + ScaleAmplitude);
      
      BounceMargin = AmplitudeFactor*2+Scale+ScaleAmplitude; // 135
      LeftBoundary = XPos + BounceMargin;
      RightBoundary = XPos + Width - BounceMargin;
     
      TiltRad = TiltDeg * deg2rad;
      lastPositionX = 0;
      lastPositionY = 0;
      PhaseVelocity = 2.5 * deg2rad;
      positionX = XPos + Width/2;
      isMovingRight = true;
    }

    float getLat(float phase, int i) {
      if(i == 0) {
        return -phase2Rad;
      } else if(i == 9) {
        return phase2Rad;
      } else {
        return -phase2Rad + phase + (i-1) * phase8Rad;
      }
    }

    void calcPoints(float phase) {
      float sin_lat[10] = {0};// = {}
      for(int i=0;i<10;i++) {
        float lat = getLat(phase, i);
        sin_lat[i] = sin( lat );
      }
      for(int j=0;j<9;j++) {
        float lon = -phase2Rad + j * phase8Rad;
        float _y = sin( lon );
        float _l = cos( lon );
        for(int i=0;i<10;i++) {
          float _x = sin_lat[i] * _l;
          points[i][j].x = _x;
          points[i][j].y = _y;
        }
      }
    }

    void tiltSphere(float ang) {
      float st = sin( ang );
      float ct = cos( ang );
      for( int i=0; i<10; i++) {
        for( int j=0; j<9; j++) {
          float _x = points[i][j].x * ct - points[i][j].y * st;
          float _y = points[i][j].x * st + points[i][j].y * ct;
          points[i][j].x = _x;
          points[i][j].y = _y;
        }
      }
    }

    float scaleTranslate(float s, float tx, float ty) {
      for( int i=0; i<10; i++) {
        for( int j=0; j<9; j++ ) {
          float _x = points[i][j].x * s + tx;
          float _y = points[i][j].y * s + ty;
          points[i][j].x = _x;
          points[i][j].y = _y;
        }
      }
    }

    void transform(float s, float tx, float ty) {
      tiltSphere( TiltRad );
      scaleTranslate( s, tx, ty );
    }

    void fillTiles(bool alter) {
      for( int j=0; j<8; j++ ) {
        for( int i=0; i<9; i++) {
          uint16_t color = alter ? RED : WHITE;
          ball.fillTriangle(points[i][j].x,     points[i][j].y,     points[i+1][j].x, points[i+1][j].y, points[i+1][j+1].x, points[i+1][j+1].y, color);
          ball.fillTriangle(points[i+1][j+1].x, points[i+1][j+1].y, points[i][j+1].x, points[i][j+1].y, points[i][j].x,     points[i][j].y, color);
          alter = !alter;
        }
      }
    }

    void drawGrid(TFT_eSprite &sprite, int x, int y, int width, int height, int spanX, int spanY) {

      int center = width / 2;
      int vspace = ( height*2 ) / Wires;
      int vpos   = height - vspace - y + YPos;
      float stepX  = width / Wires;
      float stepY  = height / Wires;
      int i, x2, centerdiff;



      for (uint16_t nPosY = 0; nPosY < height-vspace; nPosY++) {
        uint16_t nColTmp = tft.color565(nPosY % 256, 255 - (nPosY % 256), 0 );
        //sprite.drawFastHLine(0, nPosY-y, YPos+spanX, nColTmp);
        for(uint16_t nPosX = 0; nPosX < width; nPosX+=stepX/4) {
          nColTmp = tft.color565(nPosY % 256, 255 - (nPosY % 256), 255- (nPosX/2) % 256 );
          sprite.drawFastHLine(nPosX-x+XPos, nPosY-y+YPos, stepX, nColTmp);
        }
      }

      sprite.fillRect(0, vpos, width, vspace, BGColor);

      for( i=0; i<width; i+=stepX ) {
        sprite.drawFastVLine(XPos+i-x, 0, vpos, GridColor);
        centerdiff = abs(center - i);
        if(i==center) {
          x2 = i;
        } else {
          if( i < center ) {
            x2 = i - centerdiff*2;
          } else { // i > center
            x2 = i + centerdiff*2;
          }
        }
        sprite.drawLine(XPos+i-x, vpos, XPos+x2-x, YPos+vpos+vspace, GridColor);
      }
      
      for( i=0; i<height-vspace+YPos; i+=stepY ) {
        sprite.drawFastHLine(0, i-y, YPos+spanX, GridColor);
      }
      float powa = 32;
      while(powa>1) {
        powa /=2;
        sprite.drawFastHLine(0, vpos+(vspace/powa), YPos+spanX, GridColor);  
      }
      if( ShadowYPos == 0.00 ) {
        ShadowYPos = vpos + ( (vspace- shadow.height()) / 2) + YPos;
      }
    }


    void drawBall(float phase, float scale, float oldscale, float x, float y) {
      calcPoints( fmod(phase, phase8Rad) );
      transform(scale, x, y);
      fillTiles(phase >= phase8Rad);
    }



    void animate( long duration = 5000, bool clearAfter = true ) {

      AnimationDone = false;
      startedTick = millis();
      lastTick = millis();
      processTicks = 0;

      while( !AnimationDone ) {
        lastTick = millis();
        if( isMovingRight ) {
          Phase = fmod( Phase + ( phase4Rad - PhaseVelocity ), phase4Rad );
          positionX += velocityX;
        } else {
          Phase = fmod( Phase + PhaseVelocity, phase4Rad );
          positionX -= velocityX;
        }
        if ( positionX >= RightBoundary ) {
          isMovingRight = false;
          buzz_wall = true;
        } else if( positionX < LeftBoundary ) {
          isMovingRight = true;
          buzz_wall = true;
        }
        angleY = fmod( angleY + velocityY, twopi );
        float absCosAngleY = fabs( cos( angleY ) );
        variableScale = Scale + ScaleAmplitude * absCosAngleY;
        positionY = VCentering - YPosAmplitude * absCosAngleY;

        int trend = positionY - lastPositionY;
        
        if( !isMovingUp && trend < 0) {
          buzz_floor = true;
        }

        if( positionY < lastPositionY ) {
          isMovingUp = true;
        } else {
          isMovingUp = false;
        }
        
        ball.fillSprite(BGColor); // Note: Sprite is filled with black when created
        if( Wires > 0 ) {
          shadow.fillSprite( BGColor );
          int r1 = (shadow.width()/2-4)* (1-.5*absCosAngleY);
          int r2 = (shadow.height()/2-2)* (1-.5*absCosAngleY);
          //int shadowcolor = tft.color565( 0xA0, 0xA0, 0xA0 );
          
          shadow.fillEllipse( shadow.width()/2, shadow.height()/2, r1, r2, ShadowColor );
          drawGrid( shadow, positionX-spriteCenterX + shadow.width()/2, ShadowYPos+shadow.height(), Width, Height, Width, Height );
          drawGrid( ball, positionX-spriteCenterX, positionY-spriteCenterY, Width, Height, spriteWidth, spriteHeight );
          shadow.pushSprite( positionX-spriteCenterX + shadow.width()/2, ShadowYPos+shadow.height() );
        }
        drawBall( Phase, variableScale, oldScale, spriteCenterX, spriteCenterY );
        ball.pushSprite( positionX-spriteCenterX, positionY-spriteCenterY, TFT_TRANSPARENT );

        oldScale = variableScale;
        lastPositionX = positionX;
        lastPositionY = positionY;
        processTicks = millis() - lastTick;
        if( processTicks < Framelength ) {
          delay( Framelength - processTicks );
        }
        if( millis() - startedTick > duration ) {
          if( clearAfter ) {
            tft.fillRect( XPos, YPos, Width, Height, BGColor );
          }
          AnimationDone = true;
        }
      }
    }

};


AmigaRulez AmigaBall;
