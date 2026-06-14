from django.urls import path
from .views import (
    inicio,
    estado_empleados,
    detalle_empleado,
    exportar_excel,
    reporte_general,
    exportar_reporte_excel,
    recibir_datos_adms,
)

urlpatterns = [
    path('', inicio),
    path('estado/', estado_empleados),
    path('empleado/<int:empleado_id>/', detalle_empleado),
    path('reporte/', reporte_general),
    path('empleado/<int:empleado_id>/excel/', exportar_excel),
    path('reporte/excel/', exportar_reporte_excel),

    path('iclock/cdata', recibir_datos_adms),
    path('iclock/getrequest', recibir_datos_adms),
]