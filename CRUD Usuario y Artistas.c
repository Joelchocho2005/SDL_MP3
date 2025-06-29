#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//------------------*---Prototipos de funciones---------------------//
void limpiarConsola();
int menuPrincipal();
void menuArtistas();
void menuUsuarios();
void menuCanciones();
void menuAdministradores();

int main(){
    int opcion; 
    do{
        opcion=menuPrincipal();
        fflush(stdin);
        switch (opcion)
        {
        case 1: 
            menuAdministradores();

            break;
        case 2:
            menuUsuarios();
            
            break;
        case 3: 
            menuCanciones();

            
            break;
        case 4:
            menuArtistas();
            /* code */

            
            break;
        case 5:
            /* code */

            
            break;
        case 6:
            printf("Gracias por preferirnos...Saliendo..");

            
            break;
        default:
            printf("La opcion elegida es incorrecta\n");
            break;
        }   
        limpiarConsola();
    }while (opcion!=6);
    
    
    
    return 0;
}

int menuPrincipal(){
    
    int opcion;
        printf("Menu Principal\n1. Mantenimiento del administrador\n2. Mantenimiento de usuarios\n3. Mantenimiento de la Base de datos de las canciones\n");
        printf("4. Mantenimiento de Artistas\n5. Mantenimiento de Anuncios\n6. Salir\nOpcion: ");
        scanf("%d", &opcion);
    
    return opcion;
}

void limpiarConsola() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void menuAdministradores(){
    
    int opcion;
    
    do{
        limpiarConsola();
        fflush(stdin);
        printf("Menu Mantenimiento Administradores\n");
        printf("1. Agregar\n2. Modificar\n3. Eliminar\n4. Reporte de Admins\n5. Regresar\nOpcion: ");
        scanf("%d",&opcion);
        fflush(stdin);
        switch (opcion)
        {
        case 1: 
            printf("Agregar Administrador\n");
            break;
        case 2:
            printf("Modificar Administrador\n");
            break;
        case 3: 
            printf("Eliminar Administrador\n");
            break;
        case 4:
            printf("Reporte Administradores\n");
            break;
        case 5:
            printf("Redirigiendo...\n");
            break;
        default:
            printf("La opcion elegida es incorrecta\n");
            break;
        }
        system("pause");    
    }while (opcion!=5);
}

void menuArtistas(){
    int op;
    
    do{
        limpiarConsola();
        fflush(stdin);
        printf("Menu Artistas\n");
        printf("1. Agregar\n2. Modificar\n3. Eliminar\n4. Reporte de Artistas\n5. Regresar\nOpcion: ");
        scanf("%d",&op);
        fflush(stdin);
        
        switch (op)
        {
        case 1:// AGREGAR ARTISTAS
        
            break;
        case 2:// MODIFICAR ARTISTAS
            printf("Lista completa de artistas\n");
            break;

        case 3:// ELIMINAR ARTISTAS
            printf("Lista completa de artistas\n");
            break;

        case 4:// REPORTE DE ARTISTAS
            printf("Reporte de Artistas\n");
            break;
            
        case 5:
            printf("Redirigiendo...\n");
            break;
        default:
            printf("Se ha ingresado una opcion no valida\n");

            break;
        }
        system("pause");
    }while(op!=5);
}



void menuCanciones(){
    int opcion=1;
    do{
        limpiarConsola();
        fflush(stdin);
        printf("Menu Mantenimiento de la base de datos de las canciones\n");
        printf("1.Agregar Cancion\n2.Modificar Cancion\n3.Eliminar Cancion\n4.Reporte Canciones\n5.Regresar\nOpcion: ");
        scanf("%d",&opcion);
        fflush(stdin);
            switch (opcion){
            case 1:
                //TODO: Agregar Cancion  
                break;  
            case 2:
                //TODO:Modificar Cancion
                break;
            case 3:
                //TODO: Eliminar Cancion
                break;
            case 4:
                //TODO: Reporte Admins
                
                break;
            case 5:
                //TODO: Regresar
                printf("Redirigiendo...\n");
                break;
            default:
                printf("Opcion no valida\n");
                break;
    }
     system("pause"); 
    }while(opcion!=5);
}


void menuUsuarios(){
    
    int opcion;

    do{
    limpiarConsola();
    printf("Menu Usuarios\n");
    printf("1. Agregar nuevo usuario\n");
    printf("2. Modificar usuarios\n");
    printf("3. Eliminar usuario\n");
    printf("4. Reporte de usuarios\n");
    printf("5. Reporte de estadisticas\n");
    printf("6. Salir\n");
    printf("Opcion: ");
    scanf("%i", &opcion);
    fflush(stdin);
    switch (opcion){
        case 1: //AGREGA NUEVOS USUARIOS
            printf("AGREGAR USUARIO");
            break;
        case 2: //MODIFICA USUARIOS
            printf("MODIFICAR USUARIO");
            break;
        
        case 3: //ELIMINA USUARIOS
            printf("ELIMINAR USUARIO");
            break;
    
        case 4: //REPORTE DE USUARIOS
            printf("REPORTE DEUSUARIOS");
            break;
        
        case 5: //REPORTE DE ESTADISTICAS
            printf("REPORTE DE ESTADISTICAS");
            break;
        
        case 6: //SALIR
            printf("Redirigiendo...\n");
            break;   
        
        default:

            printf("Opcion no valida, intente de nuevo\n");
        break;
    }
    system("pause");
    }while(opcion != 6);
}
    