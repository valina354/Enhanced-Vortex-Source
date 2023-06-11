#ifndef G_STRING_POST_PROCESS_H
#define G_STRING_POST_PROCESS_H

#include "view_shared.h"

void PerformScenePostProcessHack();

float GetSceneFadeScalar();

void DrawBarsAndGrain(int x, int y, int w, int h);
void DrawVignette(int x, int y, int w, int h);

void SetGodraysColor(Vector col = Vector(1, 1, 1));
void SetGodraysIntensity(float i = 1.0f);
bool ShouldDrawGodrays();
void DrawGodrays(int x, int y, int w, int h);

void QueueExplosionBlur(Vector origin, float lifetime = 2.0f);
void DrawExplosionBlur(int x, int y, int w, int h);

void DrawMotionBlur(int x, int y, int w, int h);

void DrawScreenGaussianBlur(int x, int y, int w, int h);

void DrawDreamBlur(int x, int y, int w, int h, StereoEye_t stereoEye);

void DrawBloomFlare(int w, int h);

void DrawDesaturation(int x, int y, int w, int h);

void SetNightvisionParams(float flBlackFade, float flNightvisionAmount, float flOverbright);
void DrawNightvision(int x, int y, int w, int h);
float GetNightvisionMinLighting();

void DrawHurtFX(int x, int y, int w, int h);

bool ShouldDrawDoF();
void ClearDoF(int x, int y, int w, int h);
void DrawDoF(int x, int y, int w, int h, StereoEye_t stereoEye);

void ResetEffects();

#endif