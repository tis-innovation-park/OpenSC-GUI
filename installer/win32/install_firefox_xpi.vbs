' Region Description
'
' Name:
' Author:
' Version:
' Description:
' 
' 
' EndRegion

Const HKEY_LOCAL_MACHINE = &H80000002

'RegKey to search
Dim objShell
Set objShell = WScript.CreateObject("WScript.Shell")

Set WshSysEnv = objShell.Environment("PROCESS")


If WshSysEnv("ProgramFiles(x86)") = "" Then

  strInitRegDel = "SOFTWARE\Mozilla\Mozilla Firefox" 
Else

  strInitRegDel = "SOFTWARE\Wow6432Node\Mozilla\Mozilla Firefox" 
End If

'Name to Search
Const strValue = "PathToExe"

'Remote Computer. "." is local machine.
Const strComputer = "."

'ScriptPath
sPath = Replace(WScript.ScriptFullName ,WScript.ScriptName, vbNullString)
'MsgBox sPath

Set objReg = GetObject("winmgmts:{impersonationLevel=impersonate}!\\" & strComputer & "\root\default:StdRegProv")
If Len(SearchInstalldir(strInitRegDel, strValue)) > 0 Then
	szProgPath = SearchInstalldir(strInitRegDel, strValue)
        szCmdLine = Chr(34) & szProgPath & Chr(34) & " " & Chr(34) & sPath & "OpenSC_PKCS11_Module_V1.2.xpi" & Chr(34)
	objShell.Run szCmdLine
	'MsgBox szCmdLine
Else
	MsgBox "Auf ihrem Rechner ist kein Firefox installiert, das Modul kann daher nicht integriert werden." & vbcrlf & vbcrlf & "Sul suo pc non è installato Firefox. Non è stato possibile installare l’Add-On"
End If

Set objShell = Nothing
Set objReg = Nothing

Function SearchInstalldir(strRegDel, value)
	strRegSub = strRegDel
	NumbOfKeys = 0
	NumbOfAttributes = 0
	objReg.EnumKey HKEY_LOCAL_MACHINE, strRegSub, arrKeyNames
	On Error Resume Next
	NumbOfKeys = UBound(arrKeyNames)
	For i = 0 To NumbOfKeys
		regFirefoxValue = objShell.RegRead("HKLM\" & strRegSub & "\" & arrKeyNames(i) & "\Main\" & value)
		If Len(regFirefoxValue) > 0 Then
			SearchInstalldir = regFirefoxValue
			Exit For
		End If
	Next
End Function
