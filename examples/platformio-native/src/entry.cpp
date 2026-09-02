// Keep the host entry point in the same translation unit as the sketch.  The
// forward declaration makes this application's entry point strong even though
// the compatibility API declares a weak fallback main().
int main();
#include "main.cpp"

// MinGW's native PlatformIO runner uses the Windows subsystem startup object.
// This entry point keeps the example independent of the API's weak main().
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                   int nShowCmd) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nShowCmd;
  setup();
  for (;;) loop();
}
