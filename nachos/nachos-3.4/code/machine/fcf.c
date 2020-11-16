// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MaxFileLength 32

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
// Input: - User space address (int)
//
//- Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space

// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
// Advance program counters.

char *User2System(int virtAddr, int limit)
{
    int i; // index
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; //need for terminal string
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);
    //printf("\n Filename u2s:");
    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        //printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

// Ham xu ly ngoai le running time cua systemcall
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    switch (which)
    {
    case NoException:
        return;
    case PageFaultException:
        DEBUG('a', "\n\t No valid translation found");
        printf("\n\n\t No valid translation found");
        interrupt->Halt();
        break;
    case ReadOnlyException:
        DEBUG('a', "\n\t Write attempted to page marked read-only");
        printf("\n\n\t Write attempted to page marked read-only");
        interrupt->Halt();
        break;
    case BusErrorException:
        DEBUG('a', "\n\t Write attempted to page marked read-only");
        printf("\n\n\t Write attempted to page marked read-only");
        interrupt->Halt();
        break;
    case AddressErrorException:
        DEBUG('a', "\n\t Unaligned reference or one that was beyond the end of the address space");
        printf("\n\n\t Unaligned reference or one that was beyond the end of the address space");
        interrupt->Halt();
        break;
    case OverflowException:
        DEBUG('a', "\n\t Integer overflow in add or sub.");
        printf("\n\n\t Integer overflow in add or sub.");
        interrupt->Halt();
        break;
    case IllegalInstrException:
        DEBUG('a', "\n\t Unimplemented or reserved instr.");
        printf("\n\n\t Unimplemented or reserved instr.");
        interrupt->Halt();
        break;
    case NumExceptionTypes:
        DEBUG('a', "\n\t Number exception types");
        printf("\n\n\t Number exception types");
        interrupt->Halt();
        break;
    case SyscallException:
    {
        switch (type)
        {
        case SC_Halt:
        {
            DEBUG('a', "\n Shutdown, initiated by user program.");
            printf("\n\n Shutdown, initiated by user program.");
            interrupt->Halt();
            break;
        }
        case SC_Create:
        {
            int virtAddr;
            char *filename;
            DEBUG('a', "\n SC_Create call ...");
            DEBUG('a', "\n Reading virtual address of filename");
            
            virtAddr = machine->ReadRegister(4);
            DEBUG('a', "\n Reading filename.");
            
            filename = User2System(virtAddr, MaxFileLength + 1);
            if (filename == NULL)
            {
                printf("\n Not enough memory in system");
                DEBUG('a', "\n Not enough memory in system");
                machine->WriteRegister(2, -1); 
                delete filename;
                return;
            }
            DEBUG('a', "\n Finish reading filename.");
            
            if (!fileSystem->Create(filename, 0))
            {
                printf("\n Error create file '%s'", filename);
                machine->WriteRegister(2, -1);
                delete filename;
                return;
            }
            machine->WriteRegister(2, 0); 
            delete[] filename;
            break;
	  }
	default:
            printf("\n Unexpected user mode exception (%d %d)", which, type);
            interrupt->Halt();
          }
	// Advance program counters.
	registers[PrevPCReg] = registers[PCReg];	// for debugging, in case we
					// are jumping into lala-land
	registers[PCReg] = registers[NextPCReg];
	registers[NextPCReg] = pcAfter;
     }	
     }	
}