
extern int qInitResources_second_resource();

int main(int, char**)
{
  // Fails to link if the symbol is not present.
  qInitResources_second_resource();
  return 0;
}
