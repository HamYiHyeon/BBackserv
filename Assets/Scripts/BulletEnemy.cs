using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BulletEnemy : MonoBehaviour
{
    public float speed = 10.0f;
    public GameObject impactEffect;

    void Start()
    {
        // �Ѿ��� ������ �ʱ�ȭ�մϴ�. transform.forward�� ����Ͽ� ������ �������� �����մϴ�.
        GetComponent<Rigidbody>().velocity = transform.forward * speed;
    }

    void OnCollisionEnter(Collision collision)
    {
        // �浹 ȿ���� �����մϴ�.
        if (impactEffect != null)
        {
            Instantiate(impactEffect, transform.position, Quaternion.identity);
        }

        // �浹�� ���� ������Ʈ�� "Player" �±׸� ���� ��� �����մϴ�.
        if (collision.gameObject.CompareTag("Player"))
        {
            Destroy(collision.gameObject);
        }

        // �Ѿ��� �����մϴ�.
        Destroy(gameObject);
    }
}
