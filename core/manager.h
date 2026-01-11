#ifndef MANAGER_H
#define MANAGER_H

class Manager
{
public:
    virtual bool Init() = 0;
    virtual bool Start() = 0;
    virtual bool Stop() = 0;

    virtual ~Manager() {}
};

#endif // MANAGER_H