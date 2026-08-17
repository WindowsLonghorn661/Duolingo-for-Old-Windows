/*
    DUOLINGO.C
    Retro Duolingo for Windows 1.x / 3.x

    Compile using:
    Open Watcom Win16

    Lesson data:
    C:\DUO\LESSONS.DUO

    Save files:
    C:\DUO\SAVE1.DUO
    C:\DUO\SAVE2.DUO
    C:\DUO\SAVE3.DUO
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>

#define MAX_LESSONS 70
#define TOTAL_QUESTIONS 1050
#define QUESTIONS_PER_LESSON 15
#define MAX_QUESTIONS 15

#define QUESTION_OPEN 0
#define QUESTION_MC   1

#define IDC_SAVE1     1001
#define IDC_SAVE2     1002
#define IDC_SAVE3     1003

#define IDC_ANSWERBOX 1100

#define IDC_A         1201
#define IDC_B         1202
#define IDC_C         1203
#define IDC_D         1204

#define IDC_CONTINUE  1300

typedef struct
{
    int type;

    char question[256];

    char choiceA[128];
    char choiceB[128];
    char choiceC[128];
    char choiceD[128];

    char answer[128];
}
QUESTION;

typedef struct
{
    int lesson;
    int question;
    int completed;
}
SAVEFILE;

/* all questions loaded from LESSONS.DUO */

QUESTION gQuestions[QUESTIONS_PER_LESSON];

int gQuestionCount = 0;

/* current save */

SAVEFILE gSave;

int gCurrentSlot = 0;

/* retry counter */

int gFailCount = 0;

/* window handles */

HWND gMainWindow;

HWND gAnswerBox;

HWND gButtonA;
HWND gButtonB;
HWND gButtonC;
HWND gButtonD;

/* colors */

HBRUSH gGrayBrush;

COLORREF ColorGray =
    RGB(192,192,192);

COLORREF ColorBlue =
    RGB(0,0,255);

COLORREF ColorWhite =
    RGB(255,255,255);

COLORREF ColorBlack =
    RGB(0,0,0);
	
/* ===================================================== */
/* FILE PATHS                                            */
/* ===================================================== */

char gSavePath[3][32] =
{
    "SAVE1.DUO",
    "SAVE2.DUO",
    "SAVE3.DUO"
};

char gLessonFile[] =
    "LESSONS.DUO";

/* ===================================================== */
/* CREATE C:\\DUO                                         */
/* ===================================================== */

void EnsureDuoFolder(void)
{
}

/* ===================================================== */
/* RESET SAVE                                            */
/* ===================================================== */

char gBase64Table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

void Base64Encode(
    const char *input,
    char *output)
{
    strcpy(output, input);
}

void ResetSave(void)
{
    gSave.lesson = 0;
    gSave.question = 0;
    gSave.completed = 0;
}

/* ===================================================== */
/* PROGRESS                                              */
/* ===================================================== */

int ProgressPercent(void)
{
    return (int)((100.0 * gSave.completed) / TOTAL_QUESTIONS);
}

/* ===================================================== */
/* SAVE GAME                                             */
/* ===================================================== */

void SaveGame(int slot)
{
    FILE *fp;

    fp = fopen(
        gSavePath[slot],
        "w");

    if(fp == NULL)
        return;

    fprintf(
        fp,
        "LESSON=%d\n",
        gSave.lesson);

    fprintf(
        fp,
        "QUESTION=%d\n",
        gSave.question);

    fprintf(
        fp,
        "COMPLETED=%d\n",
        gSave.completed);

    fclose(fp);
}

/* ===================================================== */
/* LOAD GAME                                             */
/* ===================================================== */

void LoadGame(int slot)
{
    FILE *fp;

    ResetSave();

    fp = fopen(
        gSavePath[slot],
        "r");

    if(fp == NULL)
        return;

    fscanf(
        fp,
        "LESSON=%d\n",
        &gSave.lesson);

    fscanf(
        fp,
        "QUESTION=%d\n",
        &gSave.question);

    fscanf(
        fp,
        "COMPLETED=%d\n",
        &gSave.completed);

    fclose(fp);
}

/* ===================================================== */
/* SAVE EXISTS?                                          */
/* ===================================================== */

int SaveExists(int slot)
{
    FILE *fp;

    fp = fopen(
        gSavePath[slot],
        "r");

    if(fp == NULL)
        return 0;

    fclose(fp);

    return 1;
}

/* ===================================================== */
/* START NEW SAVE                                        */
/* ===================================================== */

void NewSave(int slot)
{
    gCurrentSlot = slot;

    ResetSave();

    SaveGame(slot);
}

/* ===================================================== */
/* LOAD EXISTING SAVE                                    */
/* ===================================================== */

void SelectSave(int slot)
{
    gCurrentSlot = slot;

    if(SaveExists(slot))
        LoadGame(slot);
    else
        NewSave(slot);

    LoadLesson(
        gSave.lesson + 1);
}

/* ===================================================== */
/* COMPLETE QUESTION                                     */
/* ===================================================== */

void CompleteQuestion(void)
{
    gSave.completed++;

    gSave.question++;

   if(gSave.question >=
   QUESTIONS_PER_LESSON)
{
    gSave.lesson++;

    gSave.question = 0;

    if(gSave.lesson < MAX_LESSONS)
    {
        LoadLesson(
            gSave.lesson + 1);
    }
}

    SaveGame(gCurrentSlot);
}

/* ===================================================== */
/* COURSE FINISHED?                                      */
/* ===================================================== */

int CourseFinished(void)
{
    return
        gSave.completed >=
        TOTAL_QUESTIONS;
}

/* ===================================================== */
/* QUESTION HELPERS                                      */
/* ===================================================== */

void ClearQuestion(QUESTION *q)
{
    q->type = QUESTION_OPEN;

    q->question[0] = 0;

    q->choiceA[0] = 0;
    q->choiceB[0] = 0;
    q->choiceC[0] = 0;
    q->choiceD[0] = 0;

    q->answer[0] = 0;
}

/* ===================================================== */
/* REMOVE NEWLINE                                        */
/* ===================================================== */

void TrimNewline(char *s)
{
    int len;

    len = strlen(s);

    while(len > 0)
    {
        if(s[len - 1] == '\n' ||
           s[len - 1] == '\r')
        {
            s[len - 1] = 0;
            len--;
        }
        else
        {
            break;
        }
    }
}

/* ===================================================== */
/* ADD OPEN QUESTION                                     */
/* ===================================================== */

void AddOpenQuestion(
    char *question,
    char *answer)
{
    QUESTION *q;

    if(gQuestionCount >= MAX_QUESTIONS)
        return;

    q = &gQuestions[gQuestionCount];

    ClearQuestion(q);

    q->type = QUESTION_OPEN;

    strcpy(q->question, question);
    strcpy(q->answer, answer);

    gQuestionCount++;
}

/* ===================================================== */
/* ADD MULTIPLE CHOICE                                   */
/* ===================================================== */

void AddMCQuestion(
    char *question,
    char *a,
    char *b,
    char *c,
    char *d,
    char *answer)
{
    QUESTION *q;

    if(gQuestionCount >= MAX_QUESTIONS)
        return;

    q = &gQuestions[gQuestionCount];

    ClearQuestion(q);

    q->type = QUESTION_MC;

    strcpy(q->question, question);

    strcpy(q->choiceA, a);
    strcpy(q->choiceB, b);
    strcpy(q->choiceC, c);
    strcpy(q->choiceD, d);

    strcpy(q->answer, answer);

    gQuestionCount++;
}

/* ===================================================== */
/* LOAD LESSON FILE                                      */
/* ===================================================== */

void LoadLesson(int lessonNumber)
{
    FILE *fp;
    char line[512];
    char lessonTag[32];
    int loading = 0;

    gQuestionCount = 0;

    wsprintf(
        lessonTag,
        "[LESSON%d]",
        lessonNumber);

    fp = fopen(
        gLessonFile,
        "r");

    if(fp == NULL)
        return;

    while(fgets(
        line,
        sizeof(line),
        fp))
    {
        char *token;

        TrimNewline(line);

        if(strcmp(line, lessonTag) == 0)
        {
            loading = 1;
            continue;
        }

        if(loading && line[0] == '[')
            break;

        if(!loading)
            continue;

        token = strtok(line, "|");

        if(token == NULL)
            continue;

        if(strcmp(token, "OPEN") == 0)
        {
            char *question;
            char *answer;

            question = strtok(NULL, "|");
            answer   = strtok(NULL, "|");

            if(question && answer)
                AddOpenQuestion(question, answer);
        }
        else if(strcmp(token, "MC") == 0)
        {
            char *question;
            char *a;
            char *b;
            char *c;
            char *d;
            char *answer;

            question = strtok(NULL, "|");
            a = strtok(NULL, "|");
            b = strtok(NULL, "|");
            c = strtok(NULL, "|");
            d = strtok(NULL, "|");
            answer = strtok(NULL, "|");

            if(question && a && b && c && d && answer)
            {
                AddMCQuestion(
                    question,
                    a,
                    b,
                    c,
                    d,
                    answer);
            }
        }
    }

    fclose(fp);
}

/* ===================================================== */
/* DEFAULT LESSON FILE                                   */
/* ===================================================== */

void CreateDefaultLessonFile(void)
{
    FILE *fp;

    fp = fopen(
        gLessonFile,
        "w");

    if(fp == NULL)
        return;

    fprintf(fp,
        "[LESSON1]\n");

    fprintf(fp,
        "OPEN|Translate: hello|hallo\n");

    fprintf(fp,
        "OPEN|Translate: yes|ja\n");

    fprintf(fp,
        "MC|What is house?|huis|hond|boom|stoel|huis\n");

    fclose(fp);
}

/* ===================================================== */
/* ENSURE LESSON FILE                                    */
/* ===================================================== */

void EnsureLessonFile(void)
{
    FILE *fp;

    fp = fopen(
        gLessonFile,
        "r");

    if(fp == NULL)
    {
        CreateDefaultLessonFile();
        return;
    }

    fclose(fp);
}

/* ===================================================== */
/* CURRENT QUESTION                                      */
/* ===================================================== */

QUESTION *GetCurrentQuestion(void)
{
    if(gSave.question < 0)
        return NULL;

    if(gSave.question >= gQuestionCount)
        return NULL;

    return &gQuestions[gSave.question];
}

/* ===================================================== */
/* CASE-INSENSITIVE COMPARE                              */
/* ===================================================== */

int SameAnswer(char *a, char *b)
{
    while(*a && *b)
    {
        char ca;
        char cb;

        ca = *a;
        cb = *b;

        if(ca >= 'A' && ca <= 'Z')
            ca += 32;

        if(cb >= 'A' && cb <= 'Z')
            cb += 32;

        if(ca != cb)
            return 0;

        a++;
        b++;
    }

    return (*a == 0 && *b == 0);
}

/* ===================================================== */
/* CORRECT POPUP                                         */
/* ===================================================== */

void ShowCorrect(HWND hwnd)
{
    MessageBeep(0xFFFFFFFF);

    MessageBox(
        hwnd,
        "Correct!",
        "Duolingo",
        MB_OK);

    gFailCount = 0;
}

/* ===================================================== */
/* INCORRECT POPUP                                       */
/* ===================================================== */

void ShowIncorrect(HWND hwnd)
{
    MessageBeep(MB_ICONHAND);

    MessageBox(
        hwnd,
        "Incorrect, Try again!",
        "Duolingo",
        MB_OK);
}

/* ===================================================== */
/* SHOW ANSWER                                           */
/* ===================================================== */

void ShowAnswer(
    HWND hwnd,
    char *answer)
{
    char buffer[256];

    wsprintf(
        buffer,
        "Incorrect, Skip\r\n\r\n%s",
        answer);

    MessageBeep(
        MB_ICONEXCLAMATION);

    MessageBox(
        hwnd,
        buffer,
        "Duolingo",
        MB_OK);
}

/* ===================================================== */
/* CHECK OPEN ANSWER                                     */
/* ===================================================== */

int CheckOpenAnswer(
    HWND hwnd,
    char *userAnswer)
{
    QUESTION *q;

    q = GetCurrentQuestion();

    if(q == NULL)
        return 0;

    if(SameAnswer(
        userAnswer,
        q->answer))
    {
        ShowCorrect(hwnd);

        CompleteQuestion();

        return 1;
    }

    gFailCount++;

    if(gFailCount >= 3)
    {
        ShowAnswer(
            hwnd,
            q->answer);

        gFailCount = 0;

        CompleteQuestion();

        return 1;
    }

    ShowIncorrect(hwnd);

    return 0;
}

/* ===================================================== */
/* CHECK MULTIPLE CHOICE                                 */
/* ===================================================== */

int CheckMCAnswer(
    HWND hwnd,
    char *selected)
{
    QUESTION *q;

    q = GetCurrentQuestion();

    if(q == NULL)
        return 0;

    if(SameAnswer(
        selected,
        q->answer))
    {
        ShowCorrect(hwnd);

        CompleteQuestion();

        return 1;
    }

    gFailCount++;

    if(gFailCount >= 3)
    {
        ShowAnswer(
            hwnd,
            q->answer);

        gFailCount = 0;

        CompleteQuestion();

        return 1;
    }

    ShowIncorrect(hwnd);

    return 0;
}

/* ===================================================== */
/* READ ANSWER BOX                                       */
/* ===================================================== */

void GetAnswerText(
    HWND hEdit,
    char *buffer,
    int size)
{
    GetWindowText(
        hEdit,
        buffer,
        size);
}

/* ===================================================== */
/* SUBMIT ANSWER                                         */
/* ===================================================== */

void SubmitAnswer(HWND hwnd)
{
    QUESTION *q;

    char text[256];

    q = GetCurrentQuestion();

    if(q == NULL)
        return;

    if(q->type == QUESTION_OPEN)
    {
        GetAnswerText(
            gAnswerBox,
            text,
            sizeof(text));

        CheckOpenAnswer(
            hwnd,
            text);
    }
}

/* ===================================================== */
/* COURSE COMPLETE                                       */
/* ===================================================== */

void ShowCompletionScreen(HWND hwnd)
{
    MessageBeep(0xFFFFFFFF);

    MessageBox(
        hwnd,
        "Congratulations!\r\n\r\nYou completed the course!",
        "Duolingo Complete",
        MB_OK);
}

/* ===================================================== */
/* SAVE SLOT WINDOW CONTROLS                             */
/* ===================================================== */

HWND gSave1Button;
HWND gSave2Button;
HWND gSave3Button;

/* ===================================================== */
/* GET SLOT LABEL                                        */
/* ===================================================== */

void GetSaveLabel(
    int slot,
    char *buffer)
{
    FILE *fp;

    int lesson;
    int question;
    int completed;

    fp = fopen(
        gSavePath[slot],
        "r");

    if(fp == NULL)
    {
        wsprintf(
            buffer,
            "New Save");
        return;
    }

    fscanf(
        fp,
        "LESSON=%d\n",
        &lesson);

    fscanf(
        fp,
        "QUESTION=%d\n",
        &question);

    fscanf(
        fp,
        "COMPLETED=%d\n",
        &completed);

    fclose(fp);

    wsprintf(
        buffer,
        "Save %d - %d%%",
        slot + 1,
        (int)((100.0 * completed) / TOTAL_QUESTIONS));
}



/* ===================================================== */
/* CREATE SAVE BUTTONS                                   */
/* ===================================================== */

void CreateSaveButtons(HWND hwnd)
{
    char text[64];

    GetSaveLabel(0, text);

    gSave1Button =
        CreateWindow(
            "BUTTON",
            text,
            WS_CHILD |
            WS_VISIBLE |
            BS_PUSHBUTTON,
            120,
            110,
            180,
            30,
            hwnd,
            (HMENU)IDC_SAVE1,
            NULL,
            NULL);

    GetSaveLabel(1, text);

    gSave2Button =
        CreateWindow(
            "BUTTON",
            text,
            WS_CHILD |
            WS_VISIBLE |
            BS_PUSHBUTTON,
            120,
            150,
            180,
            30,
            hwnd,
            (HMENU)IDC_SAVE2,
            NULL,
            NULL);

    GetSaveLabel(2, text);

    gSave3Button =
        CreateWindow(
            "BUTTON",
            text,
            WS_CHILD |
            WS_VISIBLE |
            BS_PUSHBUTTON,
            120,
            190,
            180,
            30,
            hwnd,
            (HMENU)IDC_SAVE3,
            NULL,
            NULL);
}

/* ===================================================== */
/* DRAW TITLE                                            */
/* ===================================================== */

void DrawSaveScreen(
    HWND hwnd,
    HDC hdc)
{
    HFONT hFontOld;
    HFONT hFont;

    SetBkMode(
        hdc,
        TRANSPARENT);

    SetTextColor(
        hdc,
        RGB(255,255,255));

    hFont =
        CreateFont(
            36,
            0,
            0,
            0,
            FW_BOLD,
            FALSE,
            FALSE,
            FALSE,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            FF_SWISS,
            "Arial");

    hFontOld =
        SelectObject(
            hdc,
            hFont);

    TextOut(
        hdc,
        95,
        40,
        "Duolingo",
        8);

    SelectObject(
        hdc,
        hFontOld);

    DeleteObject(
        hFont);
}

/* ===================================================== */
/* SHOW SAVE SCREEN                                      */
/* ===================================================== */

int gInSaveMenu = 1;

/* ===================================================== */
/* SAVE SLOT CHOSEN                                      */
/* ===================================================== */

void SaveSlotSelected(
    HWND hwnd,
    int slot)
{
    SelectSave(slot);

    DestroyWindow(gSave1Button);
    DestroyWindow(gSave2Button);
    DestroyWindow(gSave3Button);

    gInSaveMenu = 0;

    StartCourse(hwnd);

    InvalidateRect(
        hwnd,
        NULL,
        TRUE);
}

/* ===================================================== */
/* HANDLE SAVE BUTTONS                                   */
/* ===================================================== */

void HandleSaveCommand(
    HWND hwnd,
    WPARAM wParam)
{
    switch(LOWORD(wParam))
    {
        case IDC_SAVE1:
            SaveSlotSelected(
                hwnd,
                0);
            break;

        case IDC_SAVE2:
            SaveSlotSelected(
                hwnd,
                1);
            break;

        case IDC_SAVE3:
            SaveSlotSelected(
                hwnd,
                2);
            break;
    }
}

/* ===================================================== */
/* SAVE SCREEN PAINT                                     */
/* ===================================================== */

void PaintSaveScreen(
    HWND hwnd,
    HDC hdc)
{
    RECT r;

    GetClientRect(
        hwnd,
        &r);

    SetBkColor(
        hdc,
        RGB(0,0,255));

    FillRect(
        hdc,
        &r,
        CreateSolidBrush(
            RGB(0,0,255)));

    DrawSaveScreen(
        hwnd,
        hdc);
}

/* ===================================================== */
/* QUESTION CONTROLS                                     */
/* ===================================================== */

HWND gQuestionLabel;
HWND gProgressLabel;
HWND gSubmitButton;
HWND gLessonInfoLabel;

/* ===================================================== */
/* CREATE QUESTION UI                                    */
/* ===================================================== */

void CreateQuestionControls(HWND hwnd)
{
    gProgressLabel =
        CreateWindow(
            "STATIC",
            "",
            WS_CHILD | WS_VISIBLE,
            10,
            10,
            200,
            20,
            hwnd,
            NULL,
            NULL,
            NULL);

    gQuestionLabel =
        CreateWindow(
            "STATIC",
            "",
            WS_CHILD | WS_VISIBLE,
            20,
            50,
            450,
            40,
            hwnd,
            NULL,
            NULL,
            NULL);

    gAnswerBox =
        CreateWindow(
            "EDIT",
            "",
            WS_CHILD |
            WS_VISIBLE |
            WS_BORDER,
            20,
            110,
            250,
            25,
            hwnd,
            (HMENU)IDC_ANSWERBOX,
            NULL,
            NULL);

    gSubmitButton =
        CreateWindow(
            "BUTTON",
            "Submit",
            WS_CHILD |
            WS_VISIBLE |
            BS_PUSHBUTTON,
            290,
            110,
            90,
            25,
            hwnd,
            (HMENU)IDC_CONTINUE,
            NULL,
            NULL);
			
			gLessonInfoLabel =
    CreateWindow(
        "STATIC",
        "",
        WS_CHILD | WS_VISIBLE,
        220,
        10,
        260,
        20,
        hwnd,
        NULL,
        NULL,
        NULL);

    gButtonA =
        CreateWindow(
            "BUTTON",
            "",
            WS_CHILD |
            BS_PUSHBUTTON,
            20,
            160,
            250,
            25,
            hwnd,
            (HMENU)IDC_A,
            NULL,
            NULL);

    gButtonB =
        CreateWindow(
            "BUTTON",
            "",
            WS_CHILD |
            BS_PUSHBUTTON,
            20,
            190,
            250,
            25,
            hwnd,
            (HMENU)IDC_B,
            NULL,
            NULL);

    gButtonC =
        CreateWindow(
            "BUTTON",
            "",
            WS_CHILD |
            BS_PUSHBUTTON,
            20,
            220,
            250,
            25,
            hwnd,
            (HMENU)IDC_C,
            NULL,
            NULL);

    gButtonD =
        CreateWindow(
            "BUTTON",
            "",
            WS_CHILD |
            BS_PUSHBUTTON,
            20,
            250,
            250,
            25,
            hwnd,
            (HMENU)IDC_D,
            NULL,
            NULL);
}

/* ===================================================== */
/* HIDE ALL ANSWER CONTROLS                              */
/* ===================================================== */

void HideAnswerControls(void)
{
    ShowWindow(gAnswerBox, SW_HIDE);

    ShowWindow(gButtonA, SW_HIDE);
    ShowWindow(gButtonB, SW_HIDE);
    ShowWindow(gButtonC, SW_HIDE);
    ShowWindow(gButtonD, SW_HIDE);
}

/* ===================================================== */
/* SHOW OPEN QUESTION                                    */
/* ===================================================== */

void ShowOpenQuestion(QUESTION *q)
{
    ShowWindow(gAnswerBox, SW_SHOW);

    ShowWindow(gButtonA, SW_HIDE);
    ShowWindow(gButtonB, SW_HIDE);
    ShowWindow(gButtonC, SW_HIDE);
    ShowWindow(gButtonD, SW_HIDE);

    SetWindowText(
        gQuestionLabel,
        q->question);

    SetWindowText(
        gAnswerBox,
        "");
}

/* ===================================================== */
/* SHOW MULTIPLE CHOICE                                  */
/* ===================================================== */

void ShowMCQuestion(QUESTION *q)
{
    ShowWindow(gAnswerBox, SW_HIDE);

    ShowWindow(gButtonA, SW_SHOW);
    ShowWindow(gButtonB, SW_SHOW);
    ShowWindow(gButtonC, SW_SHOW);
    ShowWindow(gButtonD, SW_SHOW);

    SetWindowText(
        gQuestionLabel,
        q->question);

    SetWindowText(
        gButtonA,
        q->choiceA);

    SetWindowText(
        gButtonB,
        q->choiceB);

    SetWindowText(
        gButtonC,
        q->choiceC);

    SetWindowText(
        gButtonD,
        q->choiceD);
}

/* ===================================================== */
/* UPDATE PROGRESS                                       */
/* ===================================================== */

void UpdateProgressDisplay(void)
{
    char buffer[64];

    wsprintf(
        buffer,
        "Progress: %d%%",
        ProgressPercent());

    SetWindowText(
        gProgressLabel,
        buffer);
}

/* ===================================================== */
/* SHOW CURRENT QUESTION                                 */
/* ===================================================== */

void ShowCurrentQuestion(void)
{
    QUESTION *q;

    q = GetCurrentQuestion();

    if(q == NULL)
    {
        ShowCompletionScreen(
            gMainWindow);
        return;
    }

    if(gSave.lesson >= 70)
    {
        Base64Encode(q->question, q->question);
        Base64Encode(q->choiceA, q->choiceA);
        Base64Encode(q->choiceB, q->choiceB);
        Base64Encode(q->choiceC, q->choiceC);
        Base64Encode(q->choiceD, q->choiceD);
        Base64Encode(q->answer, q->answer);
    }

    UpdateProgressDisplay();
    UpdateLessonInfo();

    if(q->type == QUESTION_OPEN)
    {
        ShowOpenQuestion(q);
    }
    else
    {
        ShowMCQuestion(q);
    }
}

/* ===================================================== */
/* REFRESH AFTER ANSWER                                  */
/* ===================================================== */

void NextQuestion(void)
{
    if(CourseFinished())
    {
        ShowCompletionScreen(
            gMainWindow);
        return;
    }

    ShowCurrentQuestion();
}

/* ===================================================== */
/* HANDLE MULTIPLE CHOICE BUTTONS                        */
/* ===================================================== */

void HandleMCButton(
    HWND hwnd,
    int buttonId)
{
    QUESTION *q;

    q = GetCurrentQuestion();

    if(q == NULL)
        return;

    switch(buttonId)
    {
        case IDC_A:
            if(CheckMCAnswer(
                hwnd,
                q->choiceA))
            {
                NextQuestion();
            }
            break;

        case IDC_B:
            if(CheckMCAnswer(
                hwnd,
                q->choiceB))
            {
                NextQuestion();
            }
            break;

        case IDC_C:
            if(CheckMCAnswer(
                hwnd,
                q->choiceC))
            {
                NextQuestion();
            }
            break;

        case IDC_D:
            if(CheckMCAnswer(
                hwnd,
                q->choiceD))
            {
                NextQuestion();
            }
            break;
    }
}

/* ===================================================== */
/* HANDLE SUBMIT BUTTON                                  */
/* ===================================================== */

void HandleSubmitButton(HWND hwnd)
{
    int oldCompleted;

    oldCompleted = gSave.completed;

    SubmitAnswer(hwnd);

    if(gSave.completed != oldCompleted)
    {
        NextQuestion();
    }
}

/* ===================================================== */
/* HANDLE QUESTION COMMANDS                              */
/* ===================================================== */

void HandleQuestionCommand(
    HWND hwnd,
    WPARAM wParam)
{
    switch(LOWORD(wParam))
    {
        case IDC_CONTINUE:

            HandleSubmitButton(
                hwnd);

            break;

        case IDC_A:

            HandleMCButton(
                hwnd,
                IDC_A);

            break;

        case IDC_B:

            HandleMCButton(
                hwnd,
                IDC_B);

            break;

        case IDC_C:

            HandleMCButton(
                hwnd,
                IDC_C);

            break;

        case IDC_D:

            HandleMCButton(
                hwnd,
                IDC_D);

            break;
    }
}

/* ===================================================== */
/* ENTER KEY SUPPORT                                     */
/* ===================================================== */

void HandleEnterKey(HWND hwnd)
{
    QUESTION *q;

    q = GetCurrentQuestion();

    if(q == NULL)
        return;

    if(q->type == QUESTION_OPEN)
    {
        HandleSubmitButton(
            hwnd);
    }
}

/* ===================================================== */
/* START COURSE                                          */
/* ===================================================== */

void StartCourse(HWND hwnd)
{
    CreateQuestionControls(
        hwnd);

    ShowCurrentQuestion();
}

/* ===================================================== */
/* LESSON NUMBER                                         */
/* ===================================================== */

int CurrentLessonNumber(void)
{
    return gSave.lesson + 1;
}


/* ===================================================== */
/* QUESTION IN LESSON                                    */
/* ===================================================== */

int CurrentQuestionNumber(void)
{
    return
        (gSave.completed %
        QUESTIONS_PER_LESSON)
        + 1;
}

/* ===================================================== */
/* UPDATE WINDOW TITLE                                   */
/* ===================================================== */

void UpdateLessonTitle(HWND hwnd)
{
    char buffer[128];

    wsprintf(
        buffer,
        "Duolingo - Lesson %d - Question %d",
        CurrentLessonNumber(),
        CurrentQuestionNumber());

    SetWindowText(
        hwnd,
        buffer);
}

void UpdateLessonInfo(void)
{
    char buffer[64];
    int lessonDisplay;

    lessonDisplay = gSave.lesson + 1;

    if(lessonDisplay > MAX_LESSONS)
        lessonDisplay = MAX_LESSONS;

    wsprintf(
        buffer,
        "Lesson %d/%d  Question %d/%d",
        lessonDisplay,
        MAX_LESSONS,
        gSave.question + 1,
        QUESTIONS_PER_LESSON);

    SetWindowText(
        gLessonInfoLabel,
        buffer);
}

/* ===================================================== */
/* REFRESH GAME UI                                       */
/* ===================================================== */

void RefreshGame(void)
{
    UpdateProgressDisplay();

    UpdateLessonInfo();

    UpdateLessonTitle(
        gMainWindow);

    ShowCurrentQuestion();
}

/* ===================================================== */
/* NEXT QUESTION OR LESSON                               */
/* ===================================================== */

void AdvanceCourse(void)
{
    if(CourseFinished())
    {
        ShowCompletionScreen(
            gMainWindow);

        return;
    }

    RefreshGame();
}

/* ===================================================== */
/* PAINT MAIN WINDOW                                     */
/* ===================================================== */

void PaintMainWindow(
    HWND hwnd,
    HDC hdc)
{
    RECT r;

    HBRUSH hBrush;

    GetClientRect(
        hwnd,
        &r);

    hBrush =
        CreateSolidBrush(
            RGB(192,192,192));

    FillRect(
        hdc,
        &r,
        hBrush);

    DeleteObject(
        hBrush);

    SetTextColor(
        hdc,
        RGB(0,0,0));

    SetBkMode(
        hdc,
        TRANSPARENT);
}

/* ===================================================== */
/* WM_PAINT HANDLER                                      */
/* ===================================================== */

void HandlePaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc;

    hdc =
        BeginPaint(
            hwnd,
            &ps);

    if(gInSaveMenu)
    {
        PaintSaveScreen(
            hwnd,
            hdc);
    }
    else
    {
        PaintMainWindow(
            hwnd,
            hdc);
    }

    EndPaint(
        hwnd,
        &ps);
}

/* ===================================================== */
/* WM_COMMAND                                            */
/* ===================================================== */

void HandleCommand(
    HWND hwnd,
    WPARAM wParam)
{
    if(gInSaveMenu)
    {
        HandleSaveCommand(
            hwnd,
            wParam);
    }
    else
    {
        HandleQuestionCommand(
            hwnd,
            wParam);
    }
}

/* ===================================================== */
/* WM_KEYDOWN                                            */
/* ===================================================== */

void HandleKeyDown(
    HWND hwnd,
    WPARAM wParam)
{
    switch(wParam)
    {
        case VK_RETURN:

            HandleEnterKey(
                hwnd);

            break;
    }
}

/* ===================================================== */
/* CREATE PROGRAM                                        */
/* ===================================================== */

void InitializeProgram(HWND hwnd)
{
    gMainWindow = hwnd;

    EnsureDuoFolder();

    EnsureLessonFile();

    LoadLesson(1);

    CreateSaveButtons(
        hwnd);
}

/* ===================================================== */
/* CLEANUP                                               */
/* ===================================================== */

void CleanupProgram(void)
{
    if(gCurrentSlot >= 0 &&
       gCurrentSlot <= 2)
    {
        SaveGame(
            gCurrentSlot);
    }
}

/* ===================================================== */
/* WINDOW PROCEDURE                                      */
/* ===================================================== */

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:

            InitializeProgram(
                hwnd);

            return 0;

        case WM_COMMAND:

            HandleCommand(
                hwnd,
                wParam);

            if(!gInSaveMenu &&
               gQuestionLabel == NULL)
            {
                StartCourse(
                    hwnd);
            }

            return 0;

        case WM_KEYDOWN:

            HandleKeyDown(
                hwnd,
                wParam);

            return 0;

        case WM_PAINT:

            HandlePaint(
                hwnd);

            return 0;

        case WM_DESTROY:

            CleanupProgram();

            PostQuitMessage(
                0);

            return 0;
    }

    return DefWindowProc(
        hwnd,
        msg,
        wParam,
        lParam);
}

/* ===================================================== */
/* GLOBAL INSTANCE                                       */
/* ===================================================== */

HINSTANCE gInstance;

/* ===================================================== */
/* REGISTER WINDOW CLASS                                 */
/* ===================================================== */

int RegisterDuolingoClass(
    HINSTANCE hInst)
{
    WNDCLASS wc;

    wc.style =
        CS_HREDRAW |
        CS_VREDRAW;

    wc.lpfnWndProc =
        WndProc;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;

    wc.hInstance =
        hInst;

    wc.hIcon =
        LoadIcon(
            NULL,
            IDI_APPLICATION);

    wc.hCursor =
        LoadCursor(
            NULL,
            IDC_ARROW);

    wc.hbrBackground =
        (HBRUSH)(COLOR_WINDOW + 1);

    wc.lpszMenuName =
        NULL;

    wc.lpszClassName =
        "DUOLINGO";

    return RegisterClass(
        &wc);
}

/* ===================================================== */
/* CREATE MAIN WINDOW                                    */
/* ===================================================== */

HWND CreateMainWindow(
    HINSTANCE hInst,
    int nCmdShow)
{
    HWND hwnd;

    hwnd =
        CreateWindow(
            "DUOLINGO",
            "Duolingo",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            500,
            350,
            NULL,
            NULL,
            hInst,
            NULL);

    if(hwnd == NULL)
        return NULL;

    ShowWindow(
        hwnd,
        nCmdShow);

    UpdateWindow(
        hwnd);

    return hwnd;
}

/* ===================================================== */
/* MESSAGE LOOP                                          */
/* ===================================================== */

int RunMessageLoop(void)
{
    MSG msg;

    while(
        GetMessage(
            &msg,
            NULL,
            0,
            0))
    {
        TranslateMessage(
            &msg);

        DispatchMessage(
            &msg);
    }

    return msg.wParam;
}

/* ===================================================== */
/* APPLICATION INIT                                      */
/* ===================================================== */

int InitializeApplication(
    HINSTANCE hInst,
    int nCmdShow)
{
    if(!RegisterDuolingoClass(
        hInst))
    {
        MessageBox(
            NULL,
            "Could not register window class.",
            "Duolingo",
            MB_OK);

        return 0;
    }

    gMainWindow =
        CreateMainWindow(
            hInst,
            nCmdShow);

    if(gMainWindow == NULL)
    {
        MessageBox(
            NULL,
            "Could not create main window.",
            "Duolingo",
            MB_OK);

        return 0;
    }

    return 1;
}

/* ===================================================== */
/* MAIN ENTRY POINT                                      */
/* ===================================================== */

int PASCAL WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    gInstance =
        hInst;

    if(!InitializeApplication(
        hInst,
        nCmdShow))
    {
        return 0;
    }

    return RunMessageLoop();
}	