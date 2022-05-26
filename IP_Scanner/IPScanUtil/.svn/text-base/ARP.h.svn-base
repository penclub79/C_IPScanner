// ARP.h: interface for the CARP class.
//
// Name: CARP
// Description:	CARP class, to read ARP table by using SNMP query
//				reading implementeate in function: GetTable
//				and Add/Modify/Remove ARP entry
//				Add/Modify/Remove implementeate in function: EditEntry
// Version: 1.0.0.0
// ?992-2008 Eng. Usama El-Mokadem: musama@hotmail.com.
//
// CONTACT INFORMATION:
// Eng. Usama El-Mokadem
// Email: musama@hotmail.com
// Web: http://musama.tripod.com
// Mobile: 0020 10 1289308
// Egypt
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARP_H__894E8CE4_001A_4D91_8D28_E612579AAF18__INCLUDED_)
#define AFX_ARP_H__894E8CE4_001A_4D91_8D28_E612579AAF18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <snmp.h>

#pragma	comment(lib, "snmpapi.lib")

//-----------------------------------------------------------------------
//	From MSDN Help: http://msdn2.microsoft.com/en-us/library/aa378018(VS.85).aspx
//
//	The Microsoft SNMP service calls the SnmpExtensionInit function to initialize 
//	the SNMP extension agent DLL. This function is an element of the SNMP Extension Agent API.
//
//	BOOL SnmpExtensionInit(
//	  DWORD dwUptimeReference,                    // see NOTE below
//	  HANDLE *phSubagentTrapEvent,                // trap event handle
//	  AsnObjectIdentifier *pFirstSupportedRegion  // first MIB subtree
//	);
//-----------------------------------------------------------------------
typedef BOOL (WINAPI *PFNSNMPEXTENSIONINIT)	(DWORD, HANDLE *, AsnObjectIdentifier *);

//-----------------------------------------------------------------------
//	From MSDN Help: http://msdn2.microsoft.com/en-us/library/aa378021.aspx
//
//	The Microsoft SNMP service calls the SnmpExtensionQuery function to resolve SNMP 
//	requests that contain variables within one or more of the SNMP extension agent's 
//	registered MIB subtrees. This function is an element of the SNMP Extension Agent API. 
//
//
//	BOOL SnmpExtensionQuery(
//	  BYTE bPduType,                  // SNMPv1 PDU request type
//	  SnmpVarBindList *pVarBindList,  // pointer to variable bindings
//	  AsnInteger32 *pErrorStatus,     // pointer to SNMPv1 error status
//	  AsnInteger32 *pErrorIndex       // pointer to the error index
//	);
//-----------------------------------------------------------------------
typedef BOOL (WINAPI *PFNSNMPEXTENSIONQUERY)(BYTE, SnmpVarBindList *, AsnInteger32 *, AsnInteger32 *);

//===========================================================================
// Summary:
//     arpTable structure: Used to store ARP entries.
//===========================================================================
typedef struct
{
	unsigned long	Type;			// Type: 3:Dynamic, 4:Static
	unsigned char	IPAddress[4];	// IP Address
	unsigned char	MACAddress[6];	// MAC Address
} arpTable;

//===========================================================================
// Summary:
//     CARP class: ARP entries read and write.
//     This class allows you to read, add, modify, and remove entries
//     in ARP table, by SNMP.
//===========================================================================
class CARP  
{
private:
	HMODULE					hMIBLibrary;			// Handle for library: inetmib1.dll

	PFNSNMPEXTENSIONINIT	pfnSnmpExtensionInit;	// Pointer to function: SnmpExtensionInit
	PFNSNMPEXTENSIONQUERY	pfnSnmpExtensionQuery;	// Pointer to function: SnmpExtensionQuery

	BOOL					bInitialized;			// Flag for success Initialize

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CARP object.
	//-----------------------------------------------------------------------
	CARP();

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys CARP object, handles its cleanup.
	//-----------------------------------------------------------------------
	virtual ~CARP();

	BOOL ParseIPAddress(CString IPAddr, BYTE& nField0, BYTE& nField1, BYTE& nField2, BYTE& nField3);

	//-----------------------------------------------------------------------
	// Summary:
	//		Read ARP table for specific NIC interface.
	// Parameters:
	//		pTable			- Pointer to array of arpTable struct
	//		TableLength		- Length of the array
	//		AdapterIndex	- NIC Adapter index number
	// Returns:
	//		Number of read ARP entries
	//-----------------------------------------------------------------------
	int		GetEntries(arpTable* pTable, int TableLength, int AdapterIndex);

	//-----------------------------------------------------------------------
	// Summary:
	//		Add/Modify/Remove ARP entry for specific NIC interface.
	// Parameters:
	//	IPAddress 		- Array of 4 BYTES, 4 octs of IP Address
	//	MACAddress		- Array of 4 BYTES, 6 octs of MAC Address
	//	Type			- Entry type (2:Remove, 3:Dynamic, 4:Static)
	//	AdapterIndex	- NIC Adapter index number
	// Returns:
	//		TRUE if set successfully, FALSE otherwise.
	//-----------------------------------------------------------------------
	BOOL	EditEntry(unsigned char IPAddress[4], unsigned char MACAddress[6], unsigned long Type, int AdapterIndex);
};

#endif // !defined(AFX_ARP_H__894E8CE4_001A_4D91_8D28_E612579AAF18__INCLUDED_)
