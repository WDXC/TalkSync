#include "EchoClient.h"

int main() {
    EchoClient cli;
    cli.Start();
    cli.SendMessage("message asdjfadsl");
    return 0;
}
