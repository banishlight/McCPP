#include <network/Connection.hpp>

class AcceptChild {
    public:
        AcceptChild();
        ~AcceptChild();
        void begin();
    private:
        ConnectionList& listRef = ConnectionList::getList();
        bool running = true;
};
