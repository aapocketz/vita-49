#include <vrt/expanded_header.h>
#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char* argv[]) {

    std::string message = "hello world!";
    if (argc > 1) {
        message = std::string(argv[1]);
    }
    
    // who we sending this to
    std::string address = "127.0.0.25";
    int port = 4000;
    boost::asio::io_context io_context;
    boost::asio::ip::udp::socket socket(io_context);

    socket.open(boost::asio::ip::udp::v4());
    socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    
    // socket.set_option(boost::asio::socket_base::broadcast(true));
    boost::asio::ip::udp::endpoint senderEndpoint(boost::asio::ip::address_v4::from_string(address), port);
    
    vrt::expanded_header hdr;
    // data packet with stream ID
    hdr.header |= (VRTH_PT_IF_DATA_WITH_SID);
    hdr.stream_id = 0x64;

    // add utc timestamp
    hdr.header |= (VRTH_TSI_UTC);
    hdr.integer_secs = time(nullptr);

    // add  sample count timestamp
    hdr.header |= (VRTH_TSF_SAMPLE_CNT);
    hdr.fractional_secs = 123;

    // calculate payload size
    size_t payload_len = message.size()/4;
    size_t payload_pad = message.size() % 4;
    if (payload_pad) payload_len+=1;
    std::vector<uint32_t> payload_buf(payload_len);

    // copy into payload buffer
    strncpy((char*) &payload_buf[0], message.c_str(), message.size());
    // todo: add payload pad to class id

    // nominal header/trailer sizes
    std::vector<uint32_t> header_buf(10);
    size_t header_len;
    std::vector<uint32_t> trailer_buf(10);
    size_t trailer_len;

    vrt::expanded_header::pack(&hdr, payload_len, &header_buf[0], &header_len, &trailer_buf[0], &trailer_len);
    header_buf.resize(header_len);
    trailer_buf.resize(trailer_len);

    size_t packet_byte_size = sizeof(uint32_t) * (header_len + payload_len + trailer_len);
    printf("sending packet of size %lu\n", packet_byte_size);
    printf("payload message: %s\n", message.c_str());

    // gather udp buffers
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(header_buf));
    buffers.push_back(boost::asio::buffer(payload_buf));
    buffers.push_back(boost::asio::buffer(trailer_buf));
    socket.send_to(buffers, senderEndpoint);

    socket.close();
}