// PerlinNoise is taken from
// https://www.ics.uci.edu/~gopi/CS211B/RayTracing%20tutorial.pdf


#pragma once

struct perlin
{
  int p[512];
  perlin();
  static perlin & getInstance(){static perlin instance; return instance;}
};

static double fade(double t);
static double lerp(double t, double a, double b);
static double grad(int hash, double x, double y, double z);
double noise(double x, double y, double z);
