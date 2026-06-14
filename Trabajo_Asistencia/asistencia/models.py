from django.db import models


class Empleado(models.Model):

    nombre = models.CharField(
        max_length=150
    )

    codigo = models.CharField(
        max_length=20,
        unique=True
    )

    id_biometrico = models.IntegerField(
        unique=True
    )

    def __str__(self):
        return self.nombre


class Marcacion(models.Model):

    empleado = models.ForeignKey(
        Empleado,
        on_delete=models.CASCADE
    )

    fecha_hora = models.DateTimeField()

    tipo = models.CharField(
        max_length=30,
        choices=[
            ('ENTRADA', 'Entrada'),
            ('SALIDA', 'Salida'),
            ('SALIDA_DESCANSO', 'Salida Descanso'),
            ('ENTRADA_DESCANSO', 'Entrada Descanso'),
            ('ENTRADA_TE', 'Entrada T.E.'),
            ('SALIDA_TE', 'Salida T.E.')
        ]
    )

    def __str__(self):
        return (
            f"{self.empleado.nombre} - "
            f"{self.fecha_hora} - "
            f"{self.tipo}"
        )