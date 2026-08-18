#include "Core/DeviceIdentity.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace {
constexpr const char* DeviceIdEnvironmentVariable = "CATSRACE_DEVICE_ID";
const std::filesystem::path DeviceIdPath = std::filesystem::path("Saved") / "CatsRaceDeviceId.txt";

std::string GenerateDeviceId() {
  std::array<unsigned int, 16> Bytes{};
  std::random_device RandomDevice;
  std::uniform_int_distribution<unsigned int> Distribution(0, 255);
  for (unsigned int& Byte : Bytes) {
    Byte = Distribution(RandomDevice);
  }
  Bytes[6] = (Bytes[6] & 0x0fU) | 0x40U;
  Bytes[8] = (Bytes[8] & 0x3fU) | 0x80U;

  std::ostringstream Stream;
  Stream << std::hex << std::setfill('0');
  for (size_t Index = 0; Index < Bytes.size(); ++Index) {
    if (Index == 4 || Index == 6 || Index == 8 || Index == 10) {
      Stream << '-';
    }
    Stream << std::setw(2) << Bytes[Index];
  }
  return Stream.str();
}

std::string LoadDeviceId() {
  std::ifstream Input(DeviceIdPath);
  std::string DeviceId;
  if (Input && std::getline(Input, DeviceId) && !DeviceId.empty()) {
    return DeviceId;
  }
  return {};
}

void SaveDeviceId(const std::string& DeviceId) {
  std::error_code Error;
  std::filesystem::create_directories(DeviceIdPath.parent_path(), Error);
  std::ofstream Output(DeviceIdPath, std::ios::trunc);
  if (Output) {
    Output << DeviceId << '\n';
  }
}

std::string GetDeviceIdOverride() {
#ifdef _MSC_VER
  char* Buffer = nullptr;
  size_t BufferSize = 0;
  if (_dupenv_s(&Buffer, &BufferSize, DeviceIdEnvironmentVariable) != 0 || !Buffer) {
    return {};
  }
  const std::string Override(Buffer);
  std::free(Buffer);
  return Override;
#else
  const char* Override = std::getenv(DeviceIdEnvironmentVariable);
  return Override ? std::string(Override) : std::string{};
#endif
}
}  // namespace

std::string DeviceIdentity::GetOrCreateDeviceId() {
  static const std::string DeviceId = []() {
    const std::string Override = GetDeviceIdOverride();
    if (!Override.empty()) {
      return Override;
    }

    std::string Stored = LoadDeviceId();
    if (!Stored.empty()) {
      return Stored;
    }

    Stored = GenerateDeviceId();
    SaveDeviceId(Stored);
    return Stored;
  }();
  return DeviceId;
}
