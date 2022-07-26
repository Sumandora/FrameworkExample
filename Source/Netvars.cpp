#include "Netvars.hpp"
#include "Interfaces.hpp"
#include "Utils/VMT.hpp"
#include "SDK/ClientClass.hpp"
#include "xorstr.hpp"

#include <cstdio>
#include <cstring>
#include <map>

std::map<char*, std::map<char*, int>> netvars;

void ReadTable(RecvTable* recvTable) {
	for(int i = 0; i < recvTable->m_nProps; i++) {
		RecvProp* prop = &recvTable->m_pProps[i];
		if(!prop)
			continue;
		if(strcmp(prop->m_pVarName, xorstr_("baseclass")) == 0)
			continue;

		if(!netvars.contains(recvTable->m_pNetTableName)) // It should never already exist, but I don't trust Source
			netvars[recvTable->m_pNetTableName] = {};
			
		if(!netvars[recvTable->m_pNetTableName].contains(prop->m_pVarName))
			netvars[recvTable->m_pNetTableName][prop->m_pVarName] = prop->m_Offset;
		
		if(prop->m_pDataTable && strcmp(prop->m_pDataTable->m_pNetTableName, prop->m_pVarName) != 0) // sometimes there are tables, which have var names. They are always second; skip them
			ReadTable(prop->m_pDataTable);
	}
}

void Netvars::DumpNetvars() {
	/**
	 * GetAllClasses assembly:
	 *	mov		0x1988841(%rip),%rax	; Take RIP + offset and dereference it into RAX
	 *	push	%rbp					; Create Stackframe	|These
	 *	mov		%rsp,%rbp				; Create Stackframe	|Instructions
	 *	pop		%rbp					; Destroy Stackframe|Are Useless
	 *	mov		(%rax),%rax				; Dereference it again
	 *	ret
	 */
	
	void* getAllClasses = Utils::GetVTable(Interfaces::baseClient)[8];
	int* ripOffset = reinterpret_cast<int*>(static_cast<char*>(getAllClasses) + 3);
	void* addr = static_cast<char*>(reinterpret_cast<void*>(ripOffset)) + 4 + *ripOffset;
	addr = **reinterpret_cast<void***>(addr);

	for(ClientClass* cClass = reinterpret_cast<ClientClass*>(addr); cClass != nullptr; cClass = cClass->m_pNext) {
		RecvTable* table = cClass->m_pRecvTable;
		ReadTable(table);
	}

	// Uncomment for debugging
	// FILE* buf = fopen(xorstr_("/tmp/netvars.dmp"), "w");
	// if(buf != NULL) {
		// for (const auto& [key, value] : netvars) {
			// for (const auto& [key2, value2] : value) {
				// fprintf(buf, xorstr_("[%s][%s] = %x\n"), key, key2, value2);
			// }
		// }
		// fclose(buf);
	// }
}

int Netvars::GetOffset(const char* table, const char* name) {
	for (const auto& [key, value] : netvars) {
		if(strcmp(table, key) == 0)
			for (const auto& [key2, value2] : value) {
				if(strcmp(name, key2) == 0)
					return value2;
			}
	}
	printf(xorstr_("Couldn't find netvar %s in %s\n"), name, table);
	return 0;
}
