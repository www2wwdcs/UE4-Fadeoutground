#include "Windows.h"
#include <iostream>
#include <string>
#include <Tlhelp32.h>
#include <stdio.h>
#include <tchar.h>
#include <cstdio>
#include <locale>
using namespace std;
HANDLE ��Ϸ���;
DWORD pid;
HWND ��Ϸ���ھ��;
HANDLE ģ���ַ;
DWORD64 GName;
DWORD64 Gobject;
DWORD64 buff;
FILE* fp;
bool Read(uintptr_t addr, void* buf, int size)  //uintptr_t  �޷�������  λ������ϵͳ��
{
    ReadProcessMemory(��Ϸ���, (LPCVOID)addr, buf, size, NULL);
    return 1;
}

//ģ��
template<typename T>
T Read(uintptr_t addr)
{
    T buf;
    Read(addr, &buf, sizeof(T));
    return buf;
}
//����2
HANDLE GetProcessModuleHandle(DWORD pid, CONST TCHAR* moduleName) {
    MODULEENTRY32 moduleEntry;  //������һ���������� �������ս�����Ϣ
    HANDLE handle = NULL;
    handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);  //���н��̵ľ��
    if (!handle) {
        CloseHandle(handle); //�ͷŽ��̿��շ�ֹ�ڴ�й¶
        return NULL;
    }
    ZeroMemory(&moduleEntry, sizeof(MODULEENTRY32));
    moduleEntry.dwSize = sizeof(MODULEENTRY32);
    if (!Module32First(handle, &moduleEntry)) {
        CloseHandle(handle);
        return NULL;
    }
    do {
        if (_tcscmp(moduleEntry.szModule, moduleName) == 0) { return moduleEntry.hModule; }
    } while (Module32Next(handle, &moduleEntry)); //һ����������
    CloseHandle(handle);
    return 0;
}
bool ��ʼ��()
{
    ��Ϸ���ھ�� = FindWindowA(LPCSTR("UnrealWindow"), LPCSTR("Fadeout: Underground  "));
    //std::cout << "���ھ����" << ��Ϸ���ھ�� << std::endl;
    GetWindowThreadProcessId(��Ϸ���ھ��, &pid);
   // std::cout << "pid:" << pid << std::endl;
    ��Ϸ��� = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
    //std::cout << "��Ϸ���̾����" << ��Ϸ��� << std::endl;
    ģ���ַ = GetProcessModuleHandle(pid, L"Grapple-Win64-Shipping.exe");
    //std::cout << "ģ���ַ��" << ģ���ַ << std::endl;
    GName = (DWORD64)ģ���ַ + 0x36D52C0 ;
    //std::cout <<  std::hex<<"GName��" <<  GName << std::endl;
    Gobject = (DWORD64)ģ���ַ + 0x36EC978 +0x10;
    //std::cout <<  std::hex<<"Gobject��" << Gobject << std::endl;
    return 1;
}

struct FnameEntry
{
    uint16_t biwide : 1;
    uint16_t xxxx : 5;
    uint16_t len : 10;
};


std::string GetName(unsigned int a1)
{
   
    _int64 v1; // rbx
    int v4; // [rsp+3Ch] [rbp+14h]
    v1 = HIWORD(a1); //v1 = a1 >>16;
    v4 = (unsigned __int16)a1; //v4 = a1 & 0xffff 
    uint64_t  yy = Read<uint64_t >(GName +(v1+ 2)*8) + (2*v4);  //�ַ�����ַ
    FnameEntry  fnameEntry = Read<FnameEntry >(yy);    //�ַ�������
    char buf[1024];
    memset(buf, 0, 1024);  //��ֹ����
    Read(yy + 2,buf, fnameEntry.len) ;  //�ַ���
    std::string Name ;
    if(fnameEntry.biwide)   //��խ�ַ�ת����
    { 
        try
        {
            std::wstring_convert<  std::codecvt<wchar_t, char, mbstate_t>> converter(new std::codecvt<wchar_t, char, mbstate_t>("CHS"));   //תխ�ַ�
        }
        catch (...)
        {
            Name = buf;
        }
    }
    else
    {
        Name = buf;
    }

    size_t Pos = Name.rfind('/');    //����/
    if (Pos !=std::string::npos)
    {
        Name = Name.substr(Pos + 1);
    }
    return Name;
}


uint32_t GetNumChunks()
{
    return  Read<uint32_t >(Gobject + 0x1c);
}

uint32_t GetNumElements()
{
    return  Read<uint32_t >(Gobject + 0x14);
}

void* GetObjectFromId(size_t Id) //�������õ���Ӧ��ַ
{
    const int32_t ChunkIndex = Id / 0x10000;
    const int32_t WithinChunkIndex = Id % 0x10000;
    uint8_t* Chunks = Read<uint8_t* >(Gobject) + ChunkIndex *8;
    uint8_t* ItemPtr = Read<uint8_t*>(reinterpret_cast<uintptr_t>(Chunks));
    uint8_t* Item = ItemPtr + WithinChunkIndex * 0x18;
    return Read<void*>( reinterpret_cast<uintptr_t>(Item));
}

uint64_t GetClass(uint64_t object)
{
    uint64_t Class = Read<uint64_t>(reinterpret_cast<uintptr_t>((uint8_t*)object) + 0x10);
    return Class;
 }
uint64_t Getouter(uint64_t object)
{
    uint64_t outerClass = Read<uint64_t>(reinterpret_cast<uintptr_t>((uint8_t*)object) + 0x20);
    return outerClass;
}
uint64_t Getsclass(uint64_t object) //�����ַ
{
    uint64_t outerClass = Read<uint64_t>(reinterpret_cast<uintptr_t>((uint8_t*)object) + 0x40);
    return outerClass;
}
uint64_t Getproperties(uint64_t object)
{
    uint64_t properties = Read<uint64_t>(reinterpret_cast<uintptr_t>((uint8_t*)object) + 0x48);
    return properties;

}
uint64_t Getnext(uint64_t object)
{
    uint64_t next = Read<uint64_t>(reinterpret_cast<uintptr_t>((uint8_t*)object) + 0x28);
    return next;
}
string GetType(uint64_t object)
{
    uint64_t Type = Read<uint64_t>(reinterpret_cast<uintptr_t>((uint8_t*)object) + 0x70);
    return GetName(Read<uint64_t>(Type + 0x18)) + "*";
}
void objectdump()
{
    fopen_s(&fp, "output.txt", "w");
    for (int i = 0; i < GetNumElements(); i++)   //�������
    {

        auto object = GetObjectFromId(i);
        uint64_t Class = GetClass(reinterpret_cast<uintptr_t>(object));
        auto classname = GetName(Read<uint32_t>(Class + 0x18));
        auto name = GetName(Read<uint32_t>(reinterpret_cast<uintptr_t>(object) + 0x18));
        if (name == "" || name == "None")
        {
            continue;
        }
        for (uint64_t outer = Getouter(reinterpret_cast<uintptr_t>(object)); outer; outer = Getouter((uint64_t)outer))
        {
            name = GetName(Read<uint32_t>(outer + 0x18)) + "." + name;
        }
        fprintf(fp, "%d:[0X%p]:%s\n", i, object, (classname + "   " + name).c_str());
    }
    fclose(fp);
    std::cout << "object dump success!" << std::endl;
}
void* Getobject(string name)  //�õ��ַ�����ַ
{
    
    for (int i =0;i< GetNumElements(); i++)
    {
        auto object = GetObjectFromId(i);
        uint64_t Class = GetClass(reinterpret_cast<uintptr_t>(object));
        auto classname = GetName(Read<uint32_t>(Class + 0x18));
        auto name1 = GetName(Read<uint32_t>(reinterpret_cast<uintptr_t>(object) + 0x18));
        if (name1 == "" || name == "None")
        {
            continue;
        }
        for (uint64_t outer = Getouter(reinterpret_cast<uintptr_t>(object)); outer; outer = Getouter((uint64_t)outer))
        {
            name1 = GetName(Read<uint32_t>(outer + 0x18)) + "." + name1;
        }
        name1 = (classname + "   " + name1).c_str();
        if (name1 == name)
        {
            //std::cout << object << std::endl;
            return object;
     
        }

    }
    return 0;
    
}
bool Filterobject(uint64_t object) //���Ƿ�ʱClass��
{
    void*  obj = Getobject("Class   CoreUObject.Class");

        if (uint64_t sobj = GetClass((object)))
        {
            if ((void*)sobj == obj)
            {
                return true;
            }
            if ((void*)Getsclass((uint64_t)sobj) == obj)
            {
                return true;
            }
            return false;
        }
        else
        {
            return false;
        }

}
void ce() //��ӡ����Class���ַ
{
    for (int i = 0; i < GetNumElements(); i++)
    {
        auto object = GetObjectFromId(i);
        if (!object)continue;
        if (Filterobject(reinterpret_cast <uintptr_t>(object)))
        {
            printf("[0x%016llX]\n", (uint64_t)object);

        }
    }
}
void dumpclass(uint64_t object)
{
    string ONname; //�ֲ�����
    string classname = "class " + GetName(Read<uint64_t>(object + 0x18)); //�õ�object������
    uint64_t getsclass = Getsclass(object); //����ĵ�ַ
    if (getsclass)
    {
        classname = classname + " : public " + GetName(Read<uint64_t>(getsclass  + 0x18));   //���������
    }
    classname = classname + "\n{\n";
    //std::cout << classname <<std::endl;
    for(uint64_t obj = Getproperties(object);obj;obj= Getnext(obj))  //������Ա
    {
        string name = "  " + GetType(obj);  //�õ���Ա����
        //std::cout << name << std::endl;
        if (name.size()<70)
        {
            size_t size = 70 - name.size();
            for (size_t i = 0; i< size ;i++)
            {
                name = name + " ";
            }
        }
        name = name + GetName(Read<uint64_t>(obj + 0x18));  //�õ���Ա��
        if (name.size() < 120)
        {
            size_t size = 120 - name.size();
            for (size_t i = 0; i < size; i++)
            {
                name = name + " ";
            }
        }
        int size = Read<int>(obj + 0x34); //��Աƫ��
        int offset = Read<int>(obj + 0x44);//��Ա��С
        char buf[128]{ 0 };
        sprintf_s(buf, sizeof(buf), "//0x%.4X(0x%.4X)\n", offset, size);
        name = name + buf;
        ONname = ONname + name;
    }
    string newname = classname + ONname + "};\n\n\n";
    //std::cout << newname << std::endl;
    fprintf(fp, newname.c_str());
}
int main()
{
   ��ʼ��();
    fopen_s(&fp, "dumpsdk.txt", "w+");
    for (int i = 0; i < GetNumElements(); i++)
    {
        auto object = GetObjectFromId(i);
        if (!object)continue;
        std::cout << "������: " << i << std::endl;
        if (Filterobject(uint64_t (object)))
        {
            dumpclass(uint64_t(object));
            std::cout << "��dump " << std::endl;
        }
    }
    fclose(fp);
    
   
    return 0;
}