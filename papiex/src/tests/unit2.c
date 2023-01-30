extern void papiex_start(int point, char *label);
extern void papiex_stop(int point);

int main(int argc, char **argv)
{
  int i = 1000000;
  papiex_start(1,"Unit2 test");
  while (--i)
    usleep(1);
  papiex_stop(1);
}
