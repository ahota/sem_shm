#include "common.h"
#include "stb_image.h"

int main(int argc, char **argv) {
  Semaphore server_sem(SEMNAME_SERVER, 0);
  Semaphore client_sem(SEMNAME_CLIENT, 0);

  for (int i = 0; i < 10; i++) {
    server_sem.wait();

    SharedMemorySegment shmNumBytes(SHMNAME_HEADER, sizeof(uint64_t));

    uint64_t dataSize = 0;
    dataSize = *((uint64_t *)shmNumBytes.getData(sizeof(uint64_t)));

    // open the data segment
    SharedMemorySegment shmData(SHMNAME_DATA, dataSize);

    std::string command((const char *)shmData.getData());
    std::cout << "Server wants " << command << std::endl;
    command = "img/" + command + ".jpg";

    int width, height, nchannels;
    unsigned char *image =
        stbi_load(command.c_str(), &width, &height, &nchannels, 0);
    size_t imageBytes = width * height * nchannels * sizeof(unsigned char);
    std::vector<uint64_t> numBytesDims = {
        imageBytes, (uint64_t)width, (uint64_t)height, (uint64_t)nchannels};
    std::cout << "I'm sending " << imageBytes << " bytes" << std::endl;

    shmNumBytes.setData((void *)numBytesDims.data(),
                        numBytesDims.size() * sizeof(uint64_t));
    shmData.setData(image, imageBytes);

    stbi_image_free(image);

    client_sem.post();
  }

  return 0;
}
