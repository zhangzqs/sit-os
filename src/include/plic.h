#pragma once

void plic_init();
int plic_claim();
void plic_complete(int irq);