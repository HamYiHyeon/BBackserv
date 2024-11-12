using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerMove : MonoBehaviour
{
    public float moveSpeed = 0.1f;
    public float rotateSpeed = 0.1f; // ��ü ȸ�� �ӵ�
    public float turretRotateSpeed = 100f; // ��ž ȸ�� �ӵ�
    public float mouseSensitivity = 100f; // ���콺 ���� ���
    public Transform turret; // ��ž�� Transform �߰�

    public bool IsMoving { get; private set; } // �÷��̾��� ������ ����

    Rigidbody rb;

    void Start()
    {
        rb = GetComponent<Rigidbody>();
    }

    void Update()
    {
        IsMoving = false; // �� �����Ӹ��� �ʱ�ȭ

        // ��ũ �̵�
        float v = Input.GetAxis("Vertical");
        Vector3 dir = new Vector3(0, 0, v);
        if (v != 0) IsMoving = true; // ������ ����

        dir.Normalize();
        dir = transform.TransformDirection(dir);
        rb.MovePosition(rb.position + (dir * moveSpeed * Time.deltaTime));

        // ��ũ ��ü ȸ�� (�¿� ȭ��ǥ)
        float h = Input.GetAxis("Horizontal");
        if (h != 0) IsMoving = true; // ������ ����
        transform.Rotate(0, h * rotateSpeed * Time.deltaTime, 0);

        // ��ž ȸ�� (���콺)
        float mouseMoveX = Input.GetAxis("Mouse X");
        if (turret != null)
        {
            float turretRotationAmount = mouseMoveX * turretRotateSpeed * mouseSensitivity * Time.deltaTime;
            turret.Rotate(0, 10 * turretRotationAmount, 0);
        }
    }
}
