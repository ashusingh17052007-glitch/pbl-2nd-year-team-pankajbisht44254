class VehicleIndex {
    struct Entry { char vehicleNo[NAME_LEN]; int reservationId; Entry* next; };
    static const int BUCKETS = 101;
    Entry* table[BUCKETS];
    int count;
    unsigned int hashCode(const char* vehicleNo) const;
    Entry* find(const char* vehicleNo) const;
    VehicleIndex(const VehicleIndex&);
    VehicleIndex& operator=(const VehicleIndex&);
public:
    VehicleIndex();
    ~VehicleIndex();
    bool insert(const char* vehicleNo, int reservationId);
    bool contains(const char* vehicleNo) const;
    int  getReservationId(const char* vehicleNo) const;
    bool remove(const char* vehicleNo);
    int  size() const { return count; }
};

class ActivityLog {
    struct LogNode { char text[100]; LogNode* next; };
    LogNode* head;
    LogNode* tail;
    int count;
    ActivityLog(const ActivityLog&);
    ActivityLog& operator=(const ActivityLog&);
public:
    ActivityLog();
    ~ActivityLog();
    void log(const char* text);
    void display() const;
    int  size() const { return count; }
    const char* getAt(int index) const;
    bool saveToFile(const char* fileName) const;
    bool loadFromFile(const char* fileName);
};

class FineManager {
    int totalCollected;
    int finesIssued;
public:
    FineManager();

    int calculateFine(int minutesLate, int fineRate) const;
    int applyFine(const char* vehicleNo, int minutesLate, int fineRate, ActivityLog& log);
    int getTotalCollected() const { return totalCollected; }
    int getFinesIssued() const { return finesIssued; }
};

class AlertSystem {
public:
    bool checkCapacity(const ParkingBlock& block) const;
    void alertRepeatOffender(const char* vehicleNo, int noShows) const;
};

class ReportGenerator {
public:

    void printOccupancyReport(ParkingBlock** blocks, int n) const;
};

VehicleIndex::VehicleIndex() : count(0) {
    for (int i = 0; i < BUCKETS; ++i) table[i] = 0;
}
VehicleIndex::~VehicleIndex() {
    for (int i = 0; i < BUCKETS; ++i) {
        Entry* p = table[i];
        while (p) { Entry* old = p; p = p->next; delete old; }
    }
}
unsigned int VehicleIndex::hashCode(const char* vehicleNo) const {
    unsigned int hash = 5381;
    for (const unsigned char* p = (const unsigned char*)vehicleNo; *p; ++p)
        hash = ((hash << 5) + hash) + *p;
    return hash % BUCKETS;
}
VehicleIndex::Entry* VehicleIndex::find(const char* vehicleNo) const {
    if (!vehicleNo) return 0;
    unsigned int bucket = hashCode(vehicleNo);
    for (Entry* p = table[bucket]; p; p = p->next)
        if (strcmp(p->vehicleNo, vehicleNo) == 0) return p;
    return 0;
}
bool VehicleIndex::insert(const char* vehicleNo, int reservationId) {
    if (!vehicleNo || !vehicleNo[0] || count >= 200 || find(vehicleNo)) return false;
    unsigned int bucket = hashCode(vehicleNo);
    Entry* node = new Entry;
    copyStr(node->vehicleNo, vehicleNo, NAME_LEN);
    node->reservationId = reservationId;
    node->next = table[bucket];
    table[bucket] = node;
    ++count;
    return true;
}
bool VehicleIndex::contains(const char* vehicleNo) const { return find(vehicleNo) != 0; }
int VehicleIndex::getReservationId(const char* vehicleNo) const {
    Entry* p = find(vehicleNo);
    return p ? p->reservationId : -1;
}
bool VehicleIndex::remove(const char* vehicleNo) {
    if (!vehicleNo) return false;
    unsigned int bucket = hashCode(vehicleNo);
    Entry* p = table[bucket];
    Entry* previous = 0;
    while (p) {
        if (strcmp(p->vehicleNo, vehicleNo) == 0) {
            if (previous) previous->next = p->next; else table[bucket] = p->next;
            delete p; --count; return true;
        }
        previous = p; p = p->next;
    }
    return false;
}

ActivityLog::ActivityLog() : head(0), tail(0), count(0) {}

ActivityLog::~ActivityLog() {
    while (head) {
        LogNode* oldHead = head;
        head = head->next;
        delete oldHead;
    }
}

void ActivityLog::log(const char* text) {
    LogNode* newNode = new LogNode;
    copyStr(newNode->text, text, 100);
    newNode->next = 0;
    if (!tail) head = tail = newNode;
    else { tail->next = newNode; tail = newNode; }
    count++;
    saveToFile("activity_log.txt");
}

void ActivityLog::display() const {
    cout << "Activity log (" << count << " entries):" << endl;
    int number = 1;
    for (LogNode* p = head; p; p = p->next) cout << "  " << number++ << ". " << p->text << endl;
}
const char* ActivityLog::getAt(int index) const {
    if (index < 0) return 0;
    LogNode* p = head;
    while (p && index > 0) { p = p->next; --index; }
    return p ? p->text : 0;
}
bool ActivityLog::saveToFile(const char* fileName) const {
    if (!fileName || !fileName[0]) return false;
    ofstream file(fileName, ios::out | ios::trunc);
    if (!file) return false;
    int number = 1;
    for (LogNode* p = head; p; p = p->next)
        file << setw(4) << left << number++ << ". " << p->text << '\n';
    file.flush();
    bool good = file.good();
    file.close();
    return good;
}
bool ActivityLog::loadFromFile(const char* fileName) {
    if (!fileName || !fileName[0]) return false;
    ifstream file(fileName);
    if (!file) return false;
    char line[160];
    while (file.getline(line, sizeof(line))) {
        char* content = strchr(line, '.');
        if (content && content[1] == ' ') content += 2; else content = line;
        LogNode* node = new LogNode;
        copyStr(node->text, content, 100);
        node->next = 0;
        if (!tail) head = tail = node; else { tail->next = node; tail = node; }
        ++count;
    }
    return true;
}

FineManager::FineManager() : totalCollected(0), finesIssued(0) {}

int FineManager::calculateFine(int minutesLate, int fineRate) const {
    if (minutesLate <= 0 || fineRate <= 0) return 0;
    int blocksOf15 = (minutesLate + 14) / 15;
    return blocksOf15 * fineRate;
}

int FineManager::applyFine(const char* vehicleNo, int minutesLate, int fineRate, ActivityLog& log) {
    int fine = calculateFine(minutesLate, fineRate);
    if (fine > 0) {
        totalCollected += fine;
        finesIssued++;
        char line[100] = "FINE ";
        strncat(line, vehicleNo, 20);
        log.log(line);
    }
    return fine;
}

bool AlertSystem::checkCapacity(const ParkingBlock& block) const {
    if (block.usedCount() * 100 >= block.getCapacity() * 80) {
        cout << "[ALERT] Block " << block.getId() << " is " << block.usedCount() << "/" << block.getCapacity()
             << " full - consider redirecting new bookings." << endl;
        return true;
    }
    return false;
}
void AlertSystem::alertRepeatOffender(const char* vehicleNo, int noShows) const {
    cout << "[ALERT] Vehicle " << vehicleNo << " has " << noShows << " no-shows (repeat offender)." << endl;
}

void ReportGenerator::printOccupancyReport(ParkingBlock** blocks, int n) const {
    ParkingBlock* sortedCopy[MAX_BLOCKS];
    for (int i = 0; i < n; i++) sortedCopy[i] = blocks[i];
    BlockUtils::quickSortByOccupancy(sortedCopy, 0, n - 1);

    cout << "---- Occupancy report (most occupied first) ----" << endl;
    for (int i = 0; i < n; i++) {
        cout << "  Block " << sortedCopy[i]->getId() << "  used " << sortedCopy[i]->usedCount() << "/"
             << sortedCopy[i]->getCapacity() << "  (reserved " << sortedCopy[i]->countByState(SLOT_RESERVED)
             << ", occupied " << sortedCopy[i]->countByState(SLOT_OCCUPIED) << ")" << endl;
    }
}

enum ResStatus { RES_BOOKED, RES_CHECKED_IN, RES_EXPIRED };

