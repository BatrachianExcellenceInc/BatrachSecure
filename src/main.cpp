// vim: ts=4 sw=4 fenc=utf-8
#include <QCoreApplication>
#include "api_client.h"

using namespace BtxSecurity;

int main(int argc, char *argv[]) {
    static ApiClient *client = new ApiClient();
    delete client;
    return 0;
}

