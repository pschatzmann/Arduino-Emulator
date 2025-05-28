/// Just enough dummy functionality to make SD compile
class SPIClass {
 public:
  bool begin() { return true; }
  bool begin(int sck, int miso, int mosi, int ss) { return true; }
};

static SPIClass SPI;