/// Just enough dummy functionality to make SD compile
class SPIClass {
 public:
  bool begin() { return true; }
  bool begin(int SCK, int MISO, int MOSI, int SS) { return true; }
};

static SPIClass SPI;