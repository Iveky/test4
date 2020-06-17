// test4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
//#include <winsock2.h>
//#include <stdafx.h>
#include <afxinet.h>
#include <windows.h>
#include <string.h>



CString MakeRequestHeaders(CString& strBoundary)
{
    CString strFormat;
    CString strData;
    strFormat = _T("Content-Type: multipart/form-data; boundary=%s\r\n");
    strData.Format(strFormat, strBoundary);
    return strData;
}

CString MakePreFileData(CString& strBoundary, CString& strFolders, CString& strFileName)
{
    CString strFormat;
    CString strData;


    /*

    ostavi ovo u komentaru da ima primjer dodavanja polja s podacima

    strFormat = _T("--%s");
    strFormat += _T("\r\n");
    strFormat += _T("Content-Disposition: form-data; name=\"polje\"");
    strFormat += _T("\r\n\r\n");
    strFormat += _T("vrijednost");
    strFormat += _T("\r\n");
    
    strFormat += _T("--%s");
    strFormat += _T("\r\n");
    strFormat += _T("Content-Disposition: form-data; name=\"email\"");
    strFormat += _T("\r\n\r\n");
    strFormat += _T("emajlko");
    strFormat += _T("\r\n");
    */

    strFormat += _T("--%s");
    strFormat += _T("\r\n");
    strFormat += _T("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"");
    strFormat += _T("\r\n");
    strFormat += _T("Content-Type: application/octet-stream");
    strFormat += _T("\r\n");
    strFormat += _T("Content-Transfer-Encoding: binary");
    strFormat += _T("\r\n\r\n");

    strData.Format(strFormat, strBoundary, strFolders, strFileName);

    return strData;
}

CString MakePostFileData(CString& strBoundary)
{

    CString strFormat;
    CString strData;

    strFormat = _T("\r\n");
    strFormat += _T("--%s");
    strFormat += _T("\r\n");
    strFormat += _T("Content-Disposition: form-data; name=\"submitted\"");
    strFormat += _T("\r\n\r\n");
    strFormat += _T("");
    strFormat += _T("\r\n");
    strFormat += _T("--%s--");
    strFormat += _T("\r\n");

    strData.Format(strFormat, strBoundary, strBoundary);

    return strData;

}


int main()
{
    std::cout << "Hello World!\n";


    // Run program: Ctrl + F5 or Debug > Start Without Debugging menu
    // Debug program: F5 or Debug > Start Debugging menu

    // Tips for Getting Started: 
    //   1. Use the Solution Explorer window to add/manage files
    //   2. Use the Team Explorer window to connect to source control
    //   3. Use the Output window to see build output and other messages
    //   4. Use the Error List window to view errors
    //   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
    //   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


    DWORD dwTotalRequestLength;
    DWORD dwChunkLength;
    DWORD dwReadLength;
    DWORD dwResponseLength;
    CHttpFile* pHTTP = NULL;

    dwChunkLength = 64 * 1024 * 1024;                               //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<max. fajl je 64MB !!!
    void* pBuffer = malloc(dwChunkLength);
    CFile file;
    LPCTSTR fileName = L"C:\\Temp\\test.zip";               //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< file koji šaljemo
    CString slashedFolders = "Folder1\\Folder2";            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< folder u koji spremamo
                                                            // biti æe kreiran ispod c:\inetpub\wwwroot\KatBaApi\

    if (file.Open(fileName, CFile::modeRead | CFile::typeBinary) == FALSE)
    {
        return 0;
    }
    LPCTSTR userAgent = L"C++ client by Iveky";

    CInternetSession session(userAgent);
    CHttpConnection* connection = NULL;


        //Create the multi-part form data that goes before and after the actual file upload.
        CString nameOfFile = file.GetFileName();

        CString strHTTPBoundary = _T("--WebKitFormBoundary2305966382808");
        CString strPreFileData = MakePreFileData(strHTTPBoundary, slashedFolders, nameOfFile);
        CString strPostFileData = MakePostFileData(strHTTPBoundary);
        CString strRequestHeaders = MakeRequestHeaders(strHTTPBoundary);
        dwTotalRequestLength = strPreFileData.GetLength() + strPostFileData.GetLength() + file.GetLength();

        LPCTSTR host = L"katozor.ddns.net";          //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<adresa servera
        LPCTSTR user = L"user";                     //nebitno
        LPCTSTR pass = L"pass";                     //nebitno
        INTERNET_PORT port = 80; //20838 u razvoju  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< port servera


        connection = session.GetHttpConnection( host, port, user, pass);

        pHTTP = connection->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/KatBaApi/api/ReceiveFile"));     //<<<<<<<<<<<<<<<<<<<<<<<<<<< adresa servisa na tom serveru
        pHTTP->AddRequestHeaders(strRequestHeaders);
        pHTTP->SendRequestEx(dwTotalRequestLength, HSR_SYNC | HSR_INITIATE);

        //Write out the headers and the form variables
        //pHTTP->Write((LPSTR)(LPCSTR)strPreFileData, strPreFileData.GetLength());
        char strPre[8192];
        strcpy_s(strPre, CStringA(strPreFileData).GetString());
        UINT strPreFileDataLength = strPreFileData.GetLength();
        pHTTP->Write(strPre, strPreFileDataLength);

        //upload the file.
        dwReadLength = -1;
        //int length = file.GetLength(); //used to calculate percentage complete.
        while (0 != dwReadLength)
        {
            dwReadLength = file.Read(pBuffer, dwChunkLength);
            if (0 != dwReadLength)
            {
                pHTTP->Write(pBuffer, dwReadLength);
            }
        }

        file.Close();

        //Finish the upload.
        //pHTTP->Write((LPSTR)(LPCSTR)strPostFileData, strPostFileData.GetLength());

        char strPost[8192];
        strcpy_s(strPost, CStringA(strPostFileData).GetString());
        UINT strPostFileDataLength = strPostFileData.GetLength();
        pHTTP->Write(strPost, strPostFileDataLength);
        pHTTP->EndRequest(HSR_SYNC);


        //get the response from the server.
        LPSTR szResponse;
        CString strResponse;
        dwResponseLength = pHTTP->GetLength();
        while (0 != dwResponseLength)
        {
            szResponse = (LPSTR)malloc(dwResponseLength + 1);
            szResponse[dwResponseLength] = '\0';
            pHTTP->Read(szResponse, dwResponseLength);
            strResponse += szResponse;
            free(szResponse);
            dwResponseLength = pHTTP->GetLength();
        }

        //AfxMessageBox(strResponse);
        std::cout << strResponse << std::endl;

        //close everything up.
        pHTTP->Close();
        connection->Close();
        session.Close();


}



