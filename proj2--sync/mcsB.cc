typedef struct {
    uint8_t parentsense;
    uint8_t *parent;
    uint8_t *children[2];

    union {
        uint8_t single[4];
        uint32_t all;
    } havechild;

    union {
        uint8_t single[4];
        uint32_t all;
    } childnotready;

    uint8_t sense;
    uint8_t dummy;
    unsigned int id;
} thread_t;



/* for all threads from 0…P-1 */
t = &thread[i];
for (size_t j = 0; j < 4; j++) {
    t->havechild.single[j] = (4*i+j < (P-1)) ? true : false;
    debug ("havechild[%u]=%i\n", j, t->havechild.single[j]);
}
t->parent = (i != 0) ? &thread[(i-1)/4].childnotready.single[(i-1)%4] : &t->dummy;
t->children[0] = (2*i+1 < P) ? &thread[2*i+1].parentsense : &t->dummy;
t->children[1] = (2*i+2 < P) ? &thread[2*i+2].parentsense : &t->dummy;
t->childnotready.all = t->havechild.all;
t->parentsense = 0x0;
t->sense = 0x1;
t->dummy = 0x0;
t->id = i;

t = &thread[i];
while (t->childnotready.all != 0x0);
t->childnotready.all = t->havechild.all;
*t->parent = 0x0;
if (t->id != 0) {
    while (t->parentsense != t->sense);
}
*t->children[0] = t->sense;
*t->children[1] = t->sense;
t->sense = !t->sense;