using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Bullet : MonoBehaviour
{
    public float speed = 30.0f;
    private Vector3 direction;
    public GameObject impactEffect;

    void Start()
    {
        direction = transform.forward;
        GetComponent<Rigidbody>().velocity = direction * speed;
    }

    void OnCollisionEnter(Collision collision)
    {
        if (impactEffect != null)
        {
            Instantiate(impactEffect, transform.position, Quaternion.identity);
        }

        if (collision.gameObject.CompareTag("Enemy"))
        {
            KillCount.instance.AddKill(1);
            Destroy(collision.gameObject);
        }

        Destroy(gameObject);
    }
}
