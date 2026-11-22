VERSION 5.00
Object = "{D5C5C278-937C-4DD4-BD17-521D7E8CD6F4}#1.1#0"; "RGBCS.dll"
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   7260
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   11160
   LinkTopic       =   "Form1"
   ScaleHeight     =   7260
   ScaleWidth      =   11160
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Properties 
      Caption         =   "Properties"
      Height          =   495
      Left            =   7440
      TabIndex        =   2
      Top             =   6600
      Width           =   1575
   End
   Begin VB.CommandButton Exit 
      Caption         =   "Exit"
      Height          =   495
      Left            =   9360
      TabIndex        =   1
      Top             =   6600
      Width           =   1575
   End
   Begin RGBCSLibCtl.RGBWindow RGBWindow1 
      Height          =   6375
      Left            =   240
      OleObjectBlob   =   "Form1.frx":0000
      TabIndex        =   0
      Top             =   120
      Width           =   10695
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim hLib As New RGBEasy

Private Sub Exit_Click()
    Dim retVal As Long
    
    Call RGBWindow1.RGBCloseInput(retVal)
    Call hLib.RGBFree(retVal)
    End
End Sub

Private Sub Form_Load()
    Dim retVal As Long
        
    Call hLib.RGBLoad(retVal)
    
    If (retVal = 0) Then
        Call RGBWindow1.RGBOpenInput(0, retVal)
    End If
End Sub

Private Sub Properties_Click()
    Dim retVal As Long
    
    Call RGBWindow1.RGBInputSettings(retVal)
End Sub
