using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerMove : MonoBehaviour
{
    public float moveSpeed = 0.1f;
    public float rotateSpeed = 0.1f; // 본체 회전 속도
    public float turretRotateSpeed = 100f; // 포탑 회전 속도
    public float mouseSensitivity = 100f; // 마우스 감도 배수
    public Transform turret; // 포탑의 Transform 추가

    public bool IsMoving { get; private set; } // 플레이어의 움직임 상태

    Rigidbody rb;

    void Start()
    {
        rb = GetComponent<Rigidbody>();
    }

    void Update()
    {
        IsMoving = false; // 매 프레임마다 초기화

        // 탱크 이동
        float v = Input.GetAxis("Vertical");
        Vector3 dir = new Vector3(0, 0, v);
        if (v != 0) IsMoving = true; // 움직임 감지

        dir.Normalize();
        dir = transform.TransformDirection(dir);
        rb.MovePosition(rb.position + (dir * moveSpeed * Time.deltaTime));

        // 탱크 본체 회전 (좌우 화살표)
        float h = Input.GetAxis("Horizontal");
        if (h != 0) IsMoving = true; // 움직임 감지
        transform.Rotate(0, h * rotateSpeed * Time.deltaTime, 0);

        // 포탑 회전 (마우스)
        float mouseMoveX = Input.GetAxis("Mouse X");
        if (turret != null)
        {
            float turretRotationAmount = mouseMoveX * turretRotateSpeed * mouseSensitivity * Time.deltaTime;
            turret.Rotate(0, 10 * turretRotationAmount, 0);
        }
    }
}
