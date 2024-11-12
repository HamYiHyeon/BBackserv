using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Scroll_Track : MonoBehaviour
{
    [SerializeField]
    private float scrollSpeed = 0.05f;

    private float offset = 0.0f;
    private Renderer r;

    private PlayerMove playerController;

    void Start()
    {
        r = GetComponent<Renderer>();
        playerController = FindObjectOfType<PlayerMove>();
    }

    void Update()
    {
        if (playerController != null && playerController.IsMoving)
        {
            offset = (offset + Time.deltaTime * scrollSpeed) % 1f;
            r.material.SetTextureOffset("_MainTex", new Vector2(-1 * offset, 0f));
        }
    }
}
