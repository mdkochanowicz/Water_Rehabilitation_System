#include <wiringPiI2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <math.h>
#include <gtk/gtk.h>
#include <pthread.h>

#define ADDRESS_DEVICE 0x68 

#define POWER 0x6B
#define SAMPLE_RATE_DIVIDER 0x19
#define CONFIGURATION 0x1A
#define X_OUTPUT_HIGH_ACCELEROMETER 0x3B
#define Y_OUTPUT_HIGH_ACCELEROMETER 0x3D
#define Z_OUTPUT_HIGH_ACCELEROMETER 0x3F

#define MOTOR_PIN_1 26      /*GPIO 18 (fizyczny pin 12)*/
#define MOTOR_PIN_2 1       /*GPIO 12 (fizyczny pin 32)*/


int fileDescriptor;

int activationAngle1 = 90;      
int activationAngle2 = 0;

pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;  

GtkDrawingArea *drawingArea; 


struct ModuleData moduleData;
struct ModuleData {
    float angle;      
};

void MPU6050Initialization();
short ReadRawData(int address);
void *moduleThread(void *data);
gboolean DrawingEvent(GtkWidget *widget, cairo_t *cr);
static void ApproveButtonClick(GtkWidget *widget, gpointer data);
void ShowWindow(GtkWidget **entry1, GtkWidget **entry2);


void MPU6050Initialization() {
    wiringPiI2CWriteReg8(fileDescriptor, SAMPLE_RATE_DIVIDER, 0x07);    
    wiringPiI2CWriteReg8(fileDescriptor, POWER, 0x01);  
    wiringPiI2CWriteReg8(fileDescriptor, CONFIGURATION, 0);            
}

short ReadRawData(int address) {
    short highByte, lowByte, value;

    highByte = wiringPiI2CReadReg8(fileDescriptor, address);      
    lowByte = wiringPiI2CReadReg8(fileDescriptor, address + 1); 
    value = (highByte << 8) | lowByte;                          
    return value;
}

gboolean DrawingEvent(GtkWidget *widget, cairo_t *cr) {
    guint width, height;        
    width = gtk_widget_get_allocated_width(widget);     
    height = gtk_widget_get_allocated_height(widget); 

    double pivotX = width / 2;  
    double pivotY = height / 2;

    double endX = pivotX + -(100 * cos(moduleData.angle * G_PI / 180));
    double endY = pivotY + -(100 * sin(moduleData.angle * G_PI / 180));

    cairo_set_line_width(cr, 3);        
    cairo_move_to(cr, pivotX, pivotY); 
    cairo_line_to(cr, endX, endY);
    cairo_stroke(cr); 
    return FALSE;
}

void UpdateAnimation(float angle) {
    pthread_mutex_lock(&dataMutex);     
    
    moduleData.angle = angle;

    pthread_mutex_unlock(&dataMutex); 

    gtk_widget_queue_draw(GTK_WIDGET(drawingArea)); 
}

void CreateAnimationWindow() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);        
    gtk_window_set_title(GTK_WINDOW(window), "Animacja Ruchu Ręki");  
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 250);   

    drawingArea = GTK_DRAWING_AREA(gtk_drawing_area_new());  
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(drawingArea));
    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(DrawingEvent), NULL);    
    gtk_widget_show_all(window);    /
}

static void ApproveButtonClick(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;    
    const char *text1 = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(entries[1]));

    activationAngle1 = atoi(text1);
    activationAngle2 = atoi(text2);

    free(data);         
    gtk_main_quit(); 
}

void ShowWindow(GtkWidget **entry1, GtkWidget **entry2) {
   
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);       
    gtk_window_set_title(GTK_WINDOW(window), "Ustaw kąt aktywacji silnika");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200); 

    GtkWidget *label1 = gtk_label_new("Dolna granica");    
    *entry1 = gtk_entry_new();
    GtkWidget *label2 = gtk_label_new("Górna granica");
    *entry2 = gtk_entry_new();

    GtkWidget *approveButton = gtk_button_new_with_label("Zatwierdź");
    GtkWidget *exitButton = gtk_button_new_with_label("Zakończ");
    
    GtkWidget **entries = malloc(2 * sizeof(GtkWidget *));    
    entries[0] = *entry1;                                  
    entries[1] = *entry2;

    g_signal_connect(approveButton, "clicked", G_CALLBACK(ApproveButtonClick), entries);
    g_signal_connect(exitButton, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);  
    
    gtk_box_pack_start(GTK_BOX(box), label1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), *entry1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), label2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), *entry2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), approveButton, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), exitButton, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), box);           

    gtk_widget_show_all(window);   
    gtk_main();     
}

int main(int argc, char *argv[]) {
    gtk_init(NULL, NULL);                 
    GtkWidget *entry1 = NULL, *entry2 = NULL;   
    
    ShowWindow(&entry1, &entry2);  
    
    CreateAnimationWindow();                    

    if (wiringPiSetup() == -1) {
        printf("Błąd!");
        return 1;
    }  
     
    pthread_t thread;                    
    pthread_create(&thread, NULL, moduleThread, NULL); 

    gtk_main();  

    return 0;
}

void *moduleThread(void *data) {

    float rawAccelerationX, rawAccelerationY, rawAccelerationZ;     
    float accelerationX = 0, accelerationY = 0, accelerationZ = 0;  
    float angleY;                                                    
   
    const float RAD_TO_DEG = 180 / 3.14159; 
   
    pinMode(MOTOR_PIN_1, OUTPUT); 
    digitalWrite(MOTOR_PIN_1, LOW);     
    pinMode(MOTOR_PIN_2, OUTPUT);       
    digitalWrite(MOTOR_PIN_2, LOW);     

    fileDescriptor = wiringPiI2CSetup(ADDRESS_DEVICE);  
    MPU6050Initialization();                            

    while (1) {

        rawAccelerationX = ReadRawData(X_OUTPUT_HIGH_ACCELEROMETER);
        rawAccelerationY = ReadRawData(Y_OUTPUT_HIGH_ACCELEROMETER);
        rawAccelerationZ = ReadRawData(Z_OUTPUT_HIGH_ACCELEROMETER);

        accelerationX = rawAccelerationX / 16384.0;
        accelerationY = rawAccelerationY / 16384.0;
        accelerationZ = rawAccelerationZ / 16384.0;     

        angleY = atan2(accelerationY, accelerationZ) * RAD_TO_DEG;      

        int motor1 = 0, motor2 = 0;     

        if (angleY > activationAngle1 && angleY < 180) {
            motor1 = 1;
        }
        if (angleY > -90 && angleY < activationAngle2) {
            motor2 = 1;
        }
            if (motor1) {
                digitalWrite(MOTOR_PIN_1, HIGH);
            } else {
                digitalWrite(MOTOR_PIN_1, LOW);
            }

            if (motor2) {
                digitalWrite(MOTOR_PIN_2, HIGH);
            } else {
                digitalWrite(MOTOR_PIN_2, LOW);
            }
        UpdateAnimation(angleY);   

        printf("\nKąt ugięcia ręki względem ziemi: %.1f stopni\n", angleY);     
        delay(350); 
    }
    return NULL;
}