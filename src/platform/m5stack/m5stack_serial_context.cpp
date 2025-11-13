#include "omusubi/platform/m5stack/m5stack_serial_context.hpp"
#include <M5Stack.h>
#include <new>  // placement new用

namespace omusubi {
namespace platform {
namespace m5stack {

// ========================================
// 実装の詳細（プラットフォーム固有の型を完全に隠蔽）
// ========================================

namespace {
// 無名名前空間で実装クラスを完全に隠蔽
struct SerialImpl {
    HardwareSerial* serial;
    uint8_t port;
    uint32_t baud_rate;
    bool connected;

    explicit SerialImpl(uint8_t port_num)
        : serial(nullptr)
        , port(port_num)
        , baud_rate(115200)
        , connected(false) {
        // ポート番号に応じてHardwareSerialを割り当て
        switch (port) {
            case 0: serial = &Serial; break;
            case 1: serial = &Serial1; break;
            case 2: serial = &Serial2; break;
            default: break;
        }
    }
};

// 各ポート用の静的変数
static SerialImpl impl_port0(0);
static SerialImpl impl_port1(1);
static SerialImpl impl_port2(2);

// ポート番号から実装を取得
SerialImpl& get_impl(uint8_t port) {
    switch (port) {
        case 1: return impl_port1;
        case 2: return impl_port2;
        default: return impl_port0;
    }
}
}  // namespace

M5StackSerialContext::M5StackSerialContext(uint8_t port)
    : port_(port) {
    // 実装は静的変数なので、特に何もしない
}

M5StackSerialContext::~M5StackSerialContext() {
    SerialImpl& impl = get_impl(port_);
    if (impl.connected && impl.serial) {
        impl.serial->end();
    }
}

// ========================================
// Readable実装
// ========================================

FixedBuffer<256> M5StackSerialContext::read() {
    SerialImpl& impl = get_impl(port_);
    FixedBuffer<256> result;
    if (!impl.serial || !impl.connected) {
        return result;
    }

    // 利用可能なバイトを全て読み込む
    while (impl.serial->available() && result.size() < 256) {
        uint8_t byte = static_cast<uint8_t>(impl.serial->read());
        result.append(byte);
    }

    return result;
}

// ========================================
// Writable実装
// ========================================

void M5StackSerialContext::write(StringView text) {
    SerialImpl& impl = get_impl(port_);
    if (!impl.serial || !impl.connected) {
        return;
    }

    // StringViewの内容を書き込む
    for (uint32_t i = 0; i < text.byte_length(); ++i) {
        impl.serial->write(static_cast<uint8_t>(text[i]));
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackSerialContext::connect() {
    SerialImpl& impl = get_impl(port_);
    if (!impl.serial) {
        return false;
    }

    if (impl.connected) {
        return true;
    }

    impl.serial->begin(impl.baud_rate);
    impl.connected = true;
    return true;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
