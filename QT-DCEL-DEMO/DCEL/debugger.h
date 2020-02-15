#pragma once

void debug_proxy(const char *);

void debug_proxy(double);
void debug_proxy(int);

void debug_proxy(const char * what, double input);
void debug_proxy(const char * what, int input);

void debug_proxy(const char * what, double x, double y);