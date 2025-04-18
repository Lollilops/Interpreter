program
{

  struct ts {
    int ab;
    int ac;
  }
  ts ta, tb;
int a;
  ta.ab = 100;
  ta.ac=10;
  tb.ab =200;
  tb.ac=20;
  ta=tb;
  write(ta.ab);
  write(ta.ac);

}