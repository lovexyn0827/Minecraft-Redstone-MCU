uint8_t (func)(uint8_t par) {
  uint8_t i, x = 1 + 2, *p;
  int8_t (*(f)(uint8_t param1, uint8_t * param2))(void);
  int8_t (*(*f1)(uint8_t))(void);
  i = ((uint8_t* const *) 2 + 3) + 4 + 999(4, 5, 6) + ~0 - x++;
  while (i++ != 0) {
	x *= 3;
  }

  if (x + 2 < 99) {
	x += 99;
  }

  for (i = 0; i < 1; i++);
  for (uint8_t k = 0; k < 10; i++) {
	i *= 5;
  }

  do i++; while ((i *= 7) != 0);

  switch ((i > 3 ? (17 - 16) : x) + par) {
  case 0:
	x++;
	break;
  case 1 + 1:
	return x;
  default:
	i--;
  }

  return i * x;
  return 1 + 2 * 3 + (1 != 2) + (-3) + (1 - 1 ? 10 : 20) + 4 * sizeof(uint8_t);
}
